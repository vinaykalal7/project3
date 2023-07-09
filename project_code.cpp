#include<bits/stdc++.h>
#include "pugixml/src/pugixml.hpp"
// define
#define ll long long
#define pairldd pair<ll,pair<double,double> >

using namespace std;
using namespace pugi;

// Global variables
vector<pairldd> nodes;  //stores Id,latitute ,longitude of a node
vector<string>nodenames;  //stores their names
vector<ll>wayid;    //stores Ids of way
vector<pair<double,ll>>connect[50000];  //graph using linked list
map<ll,ll>mp;  //to map id's to integers starting from 1
map<ll,pairldd>nodemap; //to map the above integers to nodes, can be done in same map
map<ll,ll>IndtoId;

double crowflyDist(pairldd& point1,pairldd& point2);

int findNodesAndWays(){

    xml_document doc;
    // loading file, if loading fails, returns null
    if(!doc.load_file("map.osm")){
        cout<<"Can't open file\n";
        return -1;
    }

    // fchild has all children of osm which are
    // "node", "way", "relation"
    xml_node fchild=doc.child("osm");
    // iterating through all child of osm
    for(auto nodeIt: fchild){
        pair<double,double> points;
        ll id;
        string nodName = nodeIt.name();
        // if the current child is a node then add
        // access it's attributes and find id,lat and lon
        if(nodName=="node"){
            for(auto attributeIt:nodeIt.attributes()){
                string attributeName = attributeIt.name();
                if(attributeName=="id"){
                    string strId=attributeIt.value();
                    ll sz=strId.size();
                    id=0;
                    // converting parsed string to long long
                    for(ll i=0;i<sz;++i){
                        id+=(strId[i]-'0');
                        id*=10;
                    }
                    id/=10;
                }
                if(attributeName=="lat"){
                    points.first=stod(attributeIt.value());
                }    
                if(attributeName=="lon"){
                    points.second=stod(attributeIt.value());
                }        
            }
            nodes.push_back({id,points});
        }
        if(nodName=="way"){
            // if the child is a way then access its id
            for(auto attributeIt:nodeIt.attributes()){
                string str = attributeIt.name();
                if(str=="id"){
                    string strId=attributeIt.value();
                    ll sz=strId.size();
                    id=0;
                    for(ll i=0;i<sz;++i){
                        id+=(strId[i]-'0');
                        id*=10;
                    }
                    id/=10;
                }      
            }
            wayid.push_back(id);
        }
    }
    ll n=nodes.size();
    // using a map to map nodes to indices to make dijkstra simple
    for(ll i=0;i<n;++i){
        mp[nodes[i].first]=i;
        nodemap[nodes[i].first]=nodes[i];
        IndtoId[i]=nodes[i].first;
    }

    cout<<"\nTotal number of nodes in the map are : "<<nodes.size()<<'\n';
    cout<<"Total number of ways in the map are : "<<wayid.size()<<"\n\n";
    return 1;
}

// function to make a graph
void nodesInWays(){

    xml_document doc;
    if(!doc.load_file("map.osm")){
        cout<<"Can't open file\n";
        return;
    }
    auto fchild=doc.first_child();
    auto rel=fchild.first_child();
    while(rel){
        string s=rel.name();
        auto child=rel.first_child();
        // if child is way then access its child nd to
        // make a graph
        // initially make prevId=0 and when ever u encounter a new id
        // connect the id and previd and make id as prevId
        if(s=="way"){
            ll prev_id=0;
            while(child){
                string nname=child.name();
                if(nname!="nd"){
                    child=child.next_sibling();
                    continue;
                }
                ll id=0;
                string sid=child.attributes_begin()->value();
                ll sz=sid.size();
                for(ll i=0;i<sz;++i){
                    id+=(sid[i]-'0');
                    id*=10;
                }
                id/=10;
                child=child.next_sibling();
                if(prev_id!=0){
                    // if the node is already connected to some other node
                    // in the way then create an edge between those 2
                    // the weight of edge being crow fly distance
                    double dist=crowflyDist(nodemap[prev_id],nodemap[id]);
                    connect[mp[id]].push_back({dist,mp[prev_id]});
                    connect[mp[prev_id]].push_back({dist,mp[id]});
                }
                prev_id=id;
            }
        }
        if(s=="node"){
            // if the child is node access its chid which is tag
            // and find the second attributes value which will give us the name of
            // place if the first attribute value is name
            string nodename="null";
            while(child){
                auto temp=child;
                vector<string>just;
                for(auto it:temp.attributes()){
                    just.push_back(it.value());
                }
                if(just[0]=="name"){
                    nodename=just[1];
                    break;
                }
                child=child.next_sibling();
            }
            nodenames.push_back(nodename);
        }
        rel=rel.next_sibling();
    }  
}

// function to check if a string is substring of another string
// time complexity - O(size(s)*maxsizeOfstring)
int isSubstring(string s1, string s2){
    int m = s1.length();
    int n = s2.length();
    int j;
    for(int i=0;i<=n-m;i++) {
        for(j=0;j<m;j++){
            if (tolower(s2[i+j])!=tolower(s1[j]))break;
        }
        if(j==m)return 1;
    }
    return 0;
}

// Given a string matches it with any place in nodes and returns all possible
// places, time  complexity - O(size(s)*maxsizeOfstring*n) (Worst case - O)
void findPlace(){

    string s;
    cout<<"Enter a substring to match it with string in the file (case insensitive) : ";
    cin>>s;
    ll n=nodes.size();
    ll f=0;
    cout<<"Matches found are : \n";
    cout<<"ID\t\tName\n";
    for(ll i=0;i<n;++i){
        if(nodenames[i]=="null")continue;
        if(nodenames.size()>=s.size() &&  isSubstring(s,nodenames[i])){
            cout<<nodes[i].first<<'\t'<<nodenames[i]<<'\n';
            f=1;
        }
    }
    if(f==0){
        cout<<"No matches found!!\n";
        return;
    }
    return;
}

// Calculating crow fly distance using Haversine formula
double crowflyDist(pairldd& point1,pairldd& point2){

    const double R=6371000;
    double lat1=point1.second.first,lat2=point2.second.first;
    double lon1=point1.second.second,lon2=point2.second.second;

    double phi1=(lat1*M_PI)/180;
    double phi2=(lat2*M_PI)/180;

    double deltaphi=((lat2-lat1)*M_PI)/180;
    double deltalambda=((lon2-lon1)*M_PI)/180;

    double a=pow(sin(deltaphi/2),2)+cos(phi1)*cos(phi2)*pow(sin(deltalambda/2),2);

    double c=2*atan2(sqrt(a),sqrt(1-a));

    double d=abs(R*c);

    return d;
}

// function to find k-nearest nodes to a node, given it's ID
// using kth order statistics (finding nth minimum in O(n)) and then
// ordering them in O(klogk)
void kClosestNodes(){

    ll k;
    cout<<"Enter k : ";
    cin>>k;
    ll id;
    cout<<"Enter the Id of node to find k-Nearest nodes : ";
    cin>>id;
    ll index=-1;
    ll n=nodes.size();
    for(ll i=0;i<n;++i){
        if(id==nodes[i].first){
            index=i;
            break;
        }
    }
    if(index==-1){
        cout<<"Id not present in map\n";
        return;
    }
    pairldd point1=nodes[index];
    vector<pair<double,pair<ll,string> > >distance;
    for(ll i=0;i<n;++i){
        distance.push_back({crowflyDist(point1,nodes[i]),{nodes[i].first,nodenames[i]}});
    }
    nth_element(distance.begin(),distance.begin()+k,distance.end());
    vector<pair<double,pair<ll,string> > >knearest;
    for(ll i=0;i<=k;++i){
        knearest.push_back(distance[i]);
    }
    sort(knearest.begin(),knearest.end());
    cout<<"\nThe k nearest neighbours in increasing order of their distance are : \n\n";
    for(ll i=0;i<=k;++i){
        if(knearest[i].first==0)continue;
        cout<<"ID : ";
        cout<<knearest[i].second.first;
        cout<<"\t\tDistance : ";
        cout<<knearest[i].first<<" meters\t";
        cout<<"Name : ";
        cout<<knearest[i].second.second<<'\n';
    }
    return;

}

// finding shortest path using dijkstra and priority queues
// time compleity - O(V + ElogV)
void Dijkstra(){

    ll start,end;
    cout<<"Enter the start node Id : ";
    cin>>start;
    cout<<"Enter destination node Id : ";
    cin>>end;

    ll startind=mp[start];
    ll endind=mp[end];
    ll nvertex=nodes.size();
    if(mp.count(start)==0 || mp.count(end)==0){
        cout<<"Entered wrong Id's\n";
        return;
    }
    vector<double>dist(nvertex,INT_MAX);
    vector<bool>visited(nvertex,false);
    dist[startind]=0;

    priority_queue<pair<double,ll>, vector<pair<double,ll>>, greater<pair<double,ll> > >pq;
    pq.push({0,startind});
   
    vector<ll>parent(nvertex);
    parent[startind]=-1;

    while(!pq.empty()){

        ll currInd=pq.top().second;
        visited[currInd]=true;
        pq.pop();        

        for(auto it: connect[currInd]){
            double weight=it.first;
            ll ver=it.second;
            if(!visited[ver] && dist[ver]>dist[currInd]+weight){
                dist[ver]=dist[currInd]+weight;
                parent[ver]=currInd;
                pq.push({dist[ver],ver});
            }
        }
    }
    int dis=dist[endind];
    if(dist[endind]==INT_MAX){
        cout<<"NO PATH FOUND\n";
        return;
    }
    stack<ll>path;
    path.push(IndtoId[endind]);
    while(parent[endind]!=-1){
        path.push(IndtoId[endind]);
        endind=parent[endind];
    }
    path.push(IndtoId[startind]);
    cout<<"\nPath is \n";
    while(!path.empty()){
        cout<<path.top();
        if(path.size()!=1)cout<<"->";
        path.pop();
    }
    cout<<'\n';
    cout<<"Shortest distance between given source and destination is "<<dis<<" meters"<<'\n';
}

int main(){

    if(findNodesAndWays()==-1){
        return -1;
    }
    nodesInWays();
    while(1){
        cout<<"Enter 1 to find a place\n";
        cout<<"Enter 2 to find k closest nodes to a node\n";
        cout<<"Enter 3 to find shortest path between 2 nodes\n";
        cout<<"Enter 4 to exit\n";
        int input;
        cin>>input;
        if(input==1){
            findPlace();
            cout<<"-----------------------------------\n";
            cout<<'\n';
            continue;
        }
        if(input==2){
            kClosestNodes();
            cout<<"-----------------------------------\n";
            cout<<'\n';
            continue;
        }
        if(input==3){
            Dijkstra();
            cout<<"-----------------------------------\n";
            cout<<'\n';
            continue;
        }
        cout<<"Exitted Successfully!!\n";
        break;
    }
    return 0;
}