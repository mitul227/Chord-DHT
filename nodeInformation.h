#ifndef nodeInfo_h
#define nodeInfo_h

#include <iostream>
#include <vector>
#include <map>

#include "port.h"
#include "M.h"

using namespace std;

typedef long long int lli;

class NodeInformation{
	private:
		lli id;
		pair< pair<string,int> , lli > predecessor;
		pair< pair<string,int> , lli > successor;
		vector< pair< pair<string,int> , lli > > fingerTable;
		map<lli,string> dictionary;
		vector< pair< pair<string,int> , lli > > successorList;

		bool isInRing;

	public:
		SocketAndPort sp;
		
		NodeInformation();

		pair< pair<string,int> , lli > findSuccessor(lli nodeId);
		pair< pair<string,int> , lli > closestPrecedingNode(lli nodeId);
		void fixFingers();
		void stabilize();
		void notify(pair< pair<string,int> , lli > node);
		void checkPredecessor();
		void checkSuccessor();
		void updateSuccessorList();

		void printKeys();
		void storeKey(lli key,string val);
		vector< pair<lli , string> > getAllKeysForSuccessor();
		vector< pair<lli , string> > getKeysForPredecessor(lli nodeId);

		void setSuccessor(string ip,int port,lli hash);
		void setSuccessorList(string ip,int port,lli hash);
		void setPredecessor(string ip,int port,lli hash);
		void setFingerTable(string ip,int port,lli hash);
		void setId(lli id);
		void setStatus();
		
		lli getId();
		string getValue(lli key);
		vector< pair< pair<string,int> , lli > > getFingerTable();
		pair< pair<string,int> , lli > getSuccessor();
		pair< pair<string,int> , lli > getPredecessor(); 
		vector< pair< pair<string,int> , lli > > getSuccessorList();
		bool getStatus();
};

#endif