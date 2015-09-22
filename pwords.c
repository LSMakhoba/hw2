#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<pthread.h>

typedef struct dict {
  char *word;
  int count;
  struct dict *next;
} dict_t;


char *
make_word( char *word ) {
  return strcpy( malloc( strlen( word )+1 ), word );
}

dict_t *
make_dict(char *word) {
  dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
  nd->word = make_word( word );
  nd->count = 1;
  nd->next = NULL;
  return nd;
}
pthread_mutex_t mlock;
pthread_mutex_t wlock;
dict_t* d;
FILE *infile;

void
insert_word( char *word ) {
  
  //   Insert word into dict or increment count if already there
  //   return pointer to the updated dict
  
  dict_t *nd;
  dict_t *pd = NULL;		// prior to insertion point 
  dict_t *di = d;		// following insertion point
  // Search down list to find if present or point of insertion
  while(di && ( strcmp(word, di->word ) >= 0) ) { 
    if( strcmp( word, di->word ) == 0 ) { 
      di->count++;		// increment count 
      return ;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }
  nd = make_dict(word);		// not found, make entry 
  nd->next = di;		// entry bigger than word or tail 
  if (pd) {
    pd->next = nd;
    return;			// insert beond head 
  }
	d=nd;
  return;
}

void print_dict(dict_t *d){
  while (d) {
    printf("[%d] %s\n", d->count, d->word);
    d = d->next;
  }
}

int
get_word( char *buf) {//( char *buf, int n, FILE *infile) 
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) {
    if (inword && !isalpha(c)) {
      buf[inword] = '\0';	// terminate the word string
      return 1;
    } 
    if (isalpha(c)) {
      buf[inword++] = c;
    }
  }
  return 0;			// no more words
}

#define MAXWORD 1024
#define NumThreads 5
/*
typedef struct aThread {
  int file;
  double info;
} aThread_t;
*/
void* 
mythread(void* arg){

	char wordbuf[MAXWORD];
	int flag=1;

	while(flag){
		pthread_mutex_lock(&wlock);
		flag=get_word(wordbuf);
		pthread_mutex_unlock(&wlock);
		if (flag==0) break;
		pthread_mutex_lock(&mlock);
		insert_word(wordbuf);
		pthread_mutex_unlock(&mlock);
	}
pthread_exit(NULL);
}

void 
words() {
	d=NULL;
	pthread_t threads[NumThreads];
	pthread_attr_t attr;

	int threadRet, j;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&mlock,NULL);
	pthread_mutex_init(&wlock,NULL);

	for(j=0;j<NumThreads;j++){
	threadRet = pthread_create(&threads[j],&attr,mythread,NULL);
		if(threadRet){
		exit(-1);
		}
	}
	for( j=0;j<NumThreads;j++){
	pthread_join(threads[j],NULL);
	}
pthread_mutex_destroy(&wlock);
pthread_mutex_destroy(&mlock);
pthread_attr_destroy(&attr);

}

int
main( int argc, char *argv[] ) {
 
  d = NULL;
 infile = stdin;
  if (argc >= 2) {
    infile = fopen (argv[1],"r");
  }
  if( !infile ) {
    printf("Unable to open %s\n",argv[1]);
    exit( EXIT_FAILURE );
  }

	words();
	print_dict(d);
  	fclose( infile );
	return EXIT_SUCCESS;	
}

