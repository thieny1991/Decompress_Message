#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <iostream>
using namespace std;
//declaring mutex
static pthread_mutex_t bsem;
static pthread_mutex_t pbsem;
//Declaration of thread condition variable
static pthread_cond_t myturn = PTHREAD_COND_INITIALIZER;
//static int turn=0;
static pthread_cond_t parentTurn=PTHREAD_COND_INITIALIZER;

struct InfoToThread{
	vector<char> decompressedMessage;
	vector<string> bitcodes;
	vector<char> chars;
	int memberCount;
	int turn;
	//priority_queue<pthread_t>;
};

void printVectorString(vector<char> sv){
	for(char s: sv)
		cout<<s<<endl;
}

bool readInput(vector<char> &charList,vector<string>&bitcodes) {
	string ch;
	string bits;
		while ((cin.peek())!=EOF){
			if(cin.peek()=='\r'||cin.peek()=='\n'){
				cin.get();//discard newline
				continue;
			}
			//read the character, because it can be in form <EOL> so we use string type
			if(cin.peek()==' ')
				charList.push_back(' ');
			else {
				cin>>ch;
				if(ch=="<EOL>"){
					charList.push_back('\n');
				}
				else
					charList.push_back(ch[0]);
			}
			cin>>bits;
			bitcodes.push_back(bits);
		}
	}

vector<char> combine_bitString_message(char c,string bitString,vector<char> decompressedMessage){
	vector<char> newDecompressedMessage;
	int k=0;
	for (int i=0; i< bitString.length();i++){
		if(bitString[i]=='1')
			newDecompressedMessage.push_back(c);
		else{
			newDecompressedMessage.push_back(decompressedMessage[k]);
			k++;
		}
	}
	return newDecompressedMessage;
}


void childThreadPrint(char c, string bitcode){
	if(c!='\n')
		cout<<c<<" Binary code = "<<bitcode<<endl;
	else
		cout<<"<EOL>"<<" Binary code = "<<bitcode<<endl;
}
void *decompress(void *infoToThread_void_ptr){
	int member=0;
	struct InfoToThread *sharedInfo=(struct InfoToThread *)infoToThread_void_ptr;
	pthread_mutex_lock(&pbsem); //acquire lock if availble
   
	member=++sharedInfo->memberCount;
	pthread_cond_signal(&parentTurn);
	pthread_mutex_unlock(&pbsem)
	//cout<<"PPID = "<<getppid()<<" child pid ="<<getpid()<<" "<<pthread_self();
   // cout<<"  member "<< member<<endl;
	pthread_mutex_lock(&bsem);
	while(member != sharedInfo->turn)
		pthread_cond_wait(&myturn,&bsem);//wait for condition variable myturn
	//start decompressing
	sharedInfo->decompressedMessage = combine_bitString_message(sharedInfo->chars[member-1],
					sharedInfo->bitcodes[member-1],sharedInfo->decompressedMessage);
	//child thread prints out.
	childThreadPrint(sharedInfo->chars[member-1],sharedInfo->bitcodes[member-1]);
  	sharedInfo->turn--;

  	pthread_cond_broadcast(&myturn);
	pthread_mutex_unlock(&bsem);//Release the lock to be acquired by another
	                //pthread_mutex_lock or try lock call
	                //cannot make assumptions about which thread
	                //acquire the lock next
	return NULL;
}

	//----------------------------------------ENTRY TO PROGRAM --------------------------------------------------//
int main(int argc, char** argv){
	InfoToThread infos;
	int NTHREADS;
	//read the input file
    readInput(infos.chars,infos.bitcodes);
  	NTHREADS=infos.chars.size();
  	infos.turn=NTHREADS;
  	infos.memberCount=0;
  	pthread_t tid[NTHREADS];
   	pthread_mutex_init(&bsem, NULL); // Initialize access to 1, &bsem is the address of the mutex
									//NULL initialize mutex with default attributes
   		for(int i=0;i<NTHREADS;i++)
	{
		pthread_mutex_lock(&pbsem);
		if(pthread_create(&tid[i], NULL, decompress,(void *) &infos) )
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
		pthread_cond_wait(&parentTurn,&pbsem);//wait for condition variable myturn
		pthread_mutex_unlock(&pbsem);
		//while(infos.memberCount!=i+1);
	}

	// Wait for the other threads to finish.
	for (int i = 0; i < NTHREADS; i++){
        	pthread_join(tid[i], NULL);
        }


    cout<<"Decompressed file contents:\n";    
      for(char c: infos.decompressedMessage)
		cout<<c;
	cout<<endl;

    return 0;
}
