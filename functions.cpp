#include "headers.h"
#include "M.h"
#include "functions.h"
#include "helperClass.h"

typedef long long int lli;

using namespace std;

HelperFunctions help = HelperFunctions();


/* put the entered key to the proper node */
void put(string key,string value,NodeInformation &nodeInfo){
	if(key == "" || value == ""){
		cout<<"Key or value field empty\n";
		return;
	}

	else{

        lli keyHash = help.getHash(key);
		cout<<"Key is "<<key<<" and hash : "<<keyHash<<endl;

        pair< pair<string,int> , lli > node = nodeInfo.findSuccessor(keyHash);

        help.sendKeyToNode(node,keyHash,value);

        cout<<"key entered successfully\n";
	}
}

/* get key from the desired node */
void get(string key,NodeInformation nodeInfo){

    if(key == ""){
        cout<<"Key field empty\n";
        return ;
    }
    else{
        
        lli keyHash = help.getHash(key);

        pair< pair<string,int> , lli > node = nodeInfo.findSuccessor(keyHash);

        string val = help.getKeyFromNode(node,to_string(keyHash));

        if(val == "")
            cout<<"Key Not found\n";

        else
            cout<<"Found "<<key<<" : "<<val<<endl;
    }
}

/* create a new ring */
void create(NodeInformation &nodeInfo){

    string ip = nodeInfo.sp.getIpAddress();
    int port = nodeInfo.sp.getPortNumber();

    /* key to be hashed for a node is ip:port */
    string key = ip+":"+(to_string(port));   
    
    lli hash = help.getHash(key);

    /* setting id, successor , successor list , predecessor ,finger table and status of node */
    nodeInfo.setId(hash);
    nodeInfo.setSuccessor(ip,port,hash);
    nodeInfo.setSuccessorList(ip,port,hash);
    nodeInfo.setPredecessor("",-1,-1);
    nodeInfo.setFingerTable(ip,port,hash);
    nodeInfo.setStatus();

    /* launch threads,one thread will listen to request from other nodes,one will do stabilization */
    thread second(listenTo,ref(nodeInfo));
    second.detach();

    thread fifth(doStabilize,ref(nodeInfo));
    fifth.detach();

}

/* join in a DHT ring */
void join(NodeInformation &nodeInfo,string ip,string port){

    if(help.isNodeAlive(ip,atoi(port.c_str())) == false){
        cout<<"Sorry but no node is active on this ip or port\n";
        return;
    }

    /* set server socket details */
    struct sockaddr_in server;

    socklen_t l = sizeof(server);

    help.setServerDetails(server,ip,stoi(port));

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error");
        exit(-1);
    }

    string currIp = nodeInfo.sp.getIpAddress();
    string currPort = to_string(nodeInfo.sp.getPortNumber()); 

    /* generate id of current node */
    lli nodeId = help.getHash(currIp+":"+currPort);    

    char charNodeId[41];
    strcpy(charNodeId,to_string(nodeId).c_str());


    /* node sends it's id to main node to find it's successor */
    if (sendto(sock, charNodeId, strlen(charNodeId), 0, (struct sockaddr*) &server, l) == -1){
        cout<<"yaha 1\n";
        perror("error");
        exit(-1);
    }

    /* node receives id and port of it's successor */
    char ipAndPort[40];
    int len;
    if ((len = recvfrom(sock, ipAndPort, 1024, 0, (struct sockaddr *) &server, &l)) == -1){
        cout<<"yaha 2\n";
        perror("error");
        exit(-1);
    }
    ipAndPort[len] = '\0';

    close(sock);

    cout<<"Successfully joined the ring\n";

    string key = ipAndPort;
    lli hash = help.getHash(key);
    pair<string,int> ipAndPortPair = help.getIpAndPort(key);

    /* setting id, successor , successor list , predecessor, finger table and status */
    nodeInfo.setId(nodeId);
    nodeInfo.setSuccessor(ipAndPortPair.first,ipAndPortPair.second,hash);
    nodeInfo.setSuccessorList(ipAndPortPair.first,ipAndPortPair.second,hash);
    nodeInfo.setPredecessor("",-1,-1);
    nodeInfo.setFingerTable(ipAndPortPair.first,ipAndPortPair.second,hash);
    nodeInfo.setStatus();

    /* get all keys from it's successor which belongs to it now */
    help.getKeysFromSuccessor(nodeInfo , ipAndPortPair.first , ipAndPortPair.second);

    /* launch threads,one thread will listen to request from other nodes,one will do stabilization */
    thread fourth(listenTo,ref(nodeInfo));
    fourth.detach();

    thread third(doStabilize,ref(nodeInfo));
    third.detach();

}


/* print successor,predecessor,successor list and finger table of node */
void printState(NodeInformation nodeInfo){
    string ip = nodeInfo.sp.getIpAddress();
    lli id = nodeInfo.getId();
    int port = nodeInfo.sp.getPortNumber();
    vector< pair< pair<string,int> , lli > > fingerTable = nodeInfo.getFingerTable();
    cout<<"Self "<<ip<<" "<<port<<" "<<id<<endl;
    pair< pair<string,int> , lli > succ = nodeInfo.getSuccessor();
    pair< pair<string,int> , lli > pre = nodeInfo.getPredecessor();
    vector < pair< pair<string,int> , lli > > succList = nodeInfo.getSuccessorList();
    cout<<"Succ "<<succ.first.first<<" "<<succ.first.second<<" "<<succ.second<<endl;
    cout<<"Pred "<<pre.first.first<<" "<<pre.first.second<<" "<<pre.second<<endl;
    for(int i=1;i<=M;i++){
        ip = fingerTable[i].first.first;
        port = fingerTable[i].first.second;
        id = fingerTable[i].second;
        cout<<"Finger["<<i<<"] "<<id<<" "<<ip<<" "<<port<<endl;
    }
    for(int i=1;i<=R;i++){
        ip = succList[i].first.first;
        port = succList[i].first.second;
        id = succList[i].second;
        cout<<"Successor["<<i<<"] "<<id<<" "<<ip<<" "<<port<<endl;
    }
}

/* node leaves the DHT ring */
void leave(NodeInformation &nodeInfo){
    pair< pair<string,int> , lli > succ = nodeInfo.getSuccessor();
    lli id = nodeInfo.getId();

    if(id == succ.second)
        return;

    /* transfer all keys to successor before leaving the ring */

    vector< pair<lli , string> > keysAndValuesVector = nodeInfo.getAllKeysForSuccessor();

    if(keysAndValuesVector.size() == 0)
        return;

    string keysAndValues = "";

    /* will arrange all keys and val in form of key1:val1;key2:val2; */
    for(int i=0;i<keysAndValuesVector.size();i++){
        keysAndValues += to_string(keysAndValuesVector[i].first) + ":" + keysAndValuesVector[i].second;
        keysAndValues += ";";
    }

    keysAndValues += "storeKeys";

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    help.setServerDetails(serverToConnectTo,succ.first.first,succ.first.second);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error");
        exit(-1);
    }

    char keysAndValuesChar[2000];
    strcpy(keysAndValuesChar,keysAndValues.c_str());

    sendto(sock,keysAndValuesChar,strlen(keysAndValuesChar),0,(struct sockaddr *)&serverToConnectTo,l);

    close(sock);
}

/* perform different tasks according to received msg */
void doTask(NodeInformation &nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString){


    /* predecessor of this node has left the ring and has sent all it's keys to this node(it's successor) */
    if(nodeIdString.find("storeKeys") != -1){
        help.storeAllKeys(nodeInfo,nodeIdString);
    }

    /* check if the sent msg is in form of key:val, if yes then store it in current node (for put ) */
    else if(help.isKeyValue(nodeIdString)){
        pair< lli , string > keyAndVal = help.getKeyAndVal(nodeIdString);
        nodeInfo.storeKey(keyAndVal.first , keyAndVal.second);
    }

    else if(nodeIdString.find("alive") != -1){
        help.sendAcknowledgement(newSock,client);
    }

    /* contacting node wants successor list of this node */
    else if(nodeIdString.find("sendSuccList") != -1){
        help.sendSuccessorList(nodeInfo,newSock,client);
    }

    /* contacting node has just joined the ring and is asking for keys that belongs to it now */
    else if(nodeIdString.find("getKeys") != -1){
        help.sendNeccessaryKeys(nodeInfo,newSock,client,nodeIdString);
    }

    /* contacting node has run get command so send value of key it requires */
    else if(nodeIdString.find("k") != -1){
        help.sendValToNode(nodeInfo,newSock,client,nodeIdString);
    }

    /* contacting node wants the predecessor of this node */
    else if(nodeIdString.find("p") != -1){
        help.sendPredecessor(nodeInfo,newSock,client);

        /* p1 in msg means that notify the current node about this contacting node */
        if(nodeIdString.find("p1") != -1){
            callNotify(nodeInfo,nodeIdString);
        }
    }

    /* contacting node wants successor Id of this node for help in finger table */
    else if(nodeIdString.find("finger") != -1){
        help.sendSuccessorId(nodeInfo,newSock,client);
    }

    /* contacting node wants current node to find successor for it */
    else{
        help.sendSuccessor(nodeInfo,nodeIdString,newSock,client);
    }

}

/* listen to any contacting node */
void listenTo(NodeInformation &nodeInfo){
    struct sockaddr_in client;
    socklen_t l = sizeof(client);

    /* wait for any client to connect and create a new thread as soon as one connects */
    while(1){
        char charNodeId[40];
        int sock = nodeInfo.sp.getSocketFd();
        int len = recvfrom(sock, charNodeId, 1024, 0, (struct sockaddr *) &client, &l);
        charNodeId[len] = '\0';
        string nodeIdString = charNodeId;

        /* launch a thread that will perform diff tasks acc to received msg */
        thread f(doTask,ref(nodeInfo),sock,client,nodeIdString);
        f.detach();
    }
}


void doStabilize(NodeInformation &nodeInfo){

    /* do stabilize tasks */
    while(1){

        nodeInfo.checkPredecessor();

        nodeInfo.checkSuccessor();

        nodeInfo.stabilize();

        nodeInfo.updateSuccessorList();

        nodeInfo.fixFingers();

        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

/* call notify of current node which will notify curr node of contacting node */
void callNotify(NodeInformation &nodeInfo,string ipAndPort){

    ipAndPort.pop_back();
    ipAndPort.pop_back();

    /* get ip and port of client node */
    pair< string , int > ipAndPortPair = help.getIpAndPort(ipAndPort);
    string ip = ipAndPortPair.first;
    int port = ipAndPortPair.second;
    lli hash = help.getHash(ipAndPort);

    pair< pair<string,int> , lli > node;
    node.first.first = ip;
    node.first.second = port;
    node.second = hash;

    /* notify current node about this node */
    nodeInfo.notify(node);
}

/* tell about all commands */
void showHelp(){
    cout<<"1) create : will create a DHT ring\n\n";
    cout<<"2) join <ip> <port> : will join ring by connecting to main node having ip and port\n\n";
    cout<<"3) printstate : will print successor, predecessor, fingerTable and Successor list\n\n";
    cout<<"4) print : will print all keys and values present in that node\n\n";
    cout<<"5) port : will display port number on which node is listening\n\n";
    cout<<"6) port <number> : will change port number to mentioned number if that port is free\n\n";
    cout<<"7) put <key> <value> : will put key and value to the node it belongs to\n\n";
    cout<<"8) get <key> : will get value of mentioned key\n\n";
}
