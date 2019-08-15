

/*
* Client-Server In One
*/

// g++ name.cpp -o name -lpthread -lnsl
// ./name host port 
// ./name client machineName port
// port range starting 1024.
// Server also refer as host, and need 3 argument
// Client need 4 argument.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <string.h>
#include <assert.h> 
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>

#define NUM_THREADS 2
#define XMAX 80 
#define YMAX 24 
#define OBJECT_POOL_SIZE 24
#define SOCKET_ERROR        -1
#define HOST_NAME_SIZE      255
#define QUEUE_SIZE          2

//-------------------------------------------------------
//  Socket Variables 
//-------------------------------------------------------
//Globals (Server and Client)
int hSocket,hServerSocket;  /* handle to socket */
struct hostent* pHostInfo;  /* holds info about a machine */
struct sockaddr_in Address; /* Internet socket address struct */
int nAddressSize = sizeof(struct sockaddr_in);
int nHostPort;

int bulletX = -1;
int p2PosX = XMAX/2;

//Global (Client Side Only)
long nHostAddress;
unsigned nReadAmount;
char strHostName[HOST_NAME_SIZE];


//ID
std::string appID;
//-------------------------------------------------------
//  Socket Functions
//-------------------------------------------------------

bool PrompServerOrClient(int argc, char* argv[])
{
	for(int i = 0 ; i  < argc ; i ++)
		printf(" %s\n" ,  argv[i]) ;
	
    //If prompt failed.
	if(argc < 2)
    {
        printf("\nUsage: host/client host-name[client-only] host-port\n");
	    return false;
    }
	
	if (argc >= 3) //client
    {
		appID = std::string(argv[1]); //Client 
        if(appID == "client")
        {
            strcpy(strHostName, argv[2]);
            nHostPort = atoi(argv[3]);
			printf("I am client\n");
            return true;
        }
		
		if(appID == "host")
        {
            nHostPort = atoi(argv[2]);
			printf("I am host\n");
			return true;
        }

    }
	
}

bool MakingSocket()
{
	if(appID == "host")
	{
		printf("\nStarting server");

		printf("\nMaking socket\n");
		/* make a socket */
		hServerSocket = socket(AF_INET, SOCK_STREAM, 0);

		if(hServerSocket == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return false;
		}
		return true;
	}
    
    if(appID == "client")
	{
		printf("\nMaking a socket\n");
		
		/* make a socket */
		hSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(hSocket == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return false;
		}
		return true;
	}
    return false;
}

bool EstablishConnection()
{
	if(appID == "host")
	{
		/* fill address struct */
		Address.sin_addr.s_addr = INADDR_ANY;
		Address.sin_port = htons(nHostPort);
		Address.sin_family = AF_INET;
		
		/* bind to a port */
		printf("\nBinding to port %d", nHostPort);
		if(bind(hServerSocket, (struct sockaddr *) &Address, sizeof(Address)) == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			return false;
		}

		/* establish listen queue */
		printf("\nMaking a listen queue of %d elements", QUEUE_SIZE);
		if(listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR)
		{
		  printf("\nCould not listen\n");
		  return false;
		}
        
    printf("\nWaiting for a connection\n");	
     /* get the connected socket */
    hSocket = accept(hServerSocket, (struct sockaddr *) &Address,(socklen_t *) &nAddressSize);
    printf("Connected to %s:%d\n", inet_ntoa(Address.sin_addr), ntohs(Address.sin_port)); 
	return true;
	}
		
	if(appID=="client")
	{
		/* get IP address from name */
		pHostInfo = gethostbyname(strHostName);
		
		/* copy address into long */
		memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

		/* fill address struct */
		Address.sin_addr.s_addr = nHostAddress;
		Address.sin_port = htons(nHostPort);
		Address.sin_family = AF_INET;

		/* connecting to host */
		printf("\nConnecting to %s on port %d", strHostName, nHostPort);
		if(connect(hSocket, (struct sockaddr*) &Address, sizeof(Address)) == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			return false;
		}
	return true;
	}
	return false;
}

bool ClosingConnection()
{
	if(close(hSocket) == SOCKET_ERROR)
		{
			printf("\nCould not close socket\n");
			return false;
		}
}

//-------------------------------------------------------
//  Class Definitions
//-------------------------------------------------------

class Bullet
{
    private:
        bool m_active;
        int m_X;
        int m_Y; 
    public:
        Bullet() :m_active(false), m_X(-1), m_Y(-1) {}
        ~Bullet() {}
		int GetX() {return m_X;}
        void SetX(int i) { m_X = i;}
		int GetY() {return m_Y;}
        void SetY(int i) { m_Y = i;}
		bool GetActive() { return m_active;}
        void SetActive(bool b) { m_active = b;}
        void CapValue(int Ymin, int Ymax)
        { }
};

class Spaceship 
{
    private:
        std::string m_ID;
        Bullet* bulletObjectPool[OBJECT_POOL_SIZE];
        int m_X;
        int m_Y;
		bool m_alive;
    public:
        Spaceship() :m_X(-1), m_Y(-1), m_alive(true)
        { 
            for(int i = 0 ; i < OBJECT_POOL_SIZE; ++i)
			{
                bulletObjectPool[i] = new Bullet();
				bulletObjectPool[i]->SetActive(false);
				bulletObjectPool[i]->SetX(-1);
				bulletObjectPool[i]->SetY(-1);
			}
        }
        
        ~Spaceship()
        {
            for(int i = 0 ; i < OBJECT_POOL_SIZE ; ++i)
                delete bulletObjectPool[i];
        }
        
        void SetID(std::string s) { m_ID = s;}
		void SetCoordinate(int x, int y) { m_X = x; m_Y = y; }
        void SetX(int i) { m_X = i;}
        void SetY(int i) { m_Y = i;}
		
        std::string GetID() {return m_ID;}
        int GetX() {return m_X;}
        int GetY() {return m_Y;}
		
		void SetAliveFalse() { m_alive = false; }
		bool GetAliveState() { return m_alive; }
        
        void CapValue(int Xmin, int Xmax, int Ymin, int Ymax)
        {
            if(m_X <= Xmin )
                m_X = Xmin+1;
            if(m_X >= Xmax )
                m_X = Xmax-1;
       
            if(m_Y <= Ymin)
                m_Y = Ymin;
            if(m_Y >= Ymax)
                m_Y = Ymax-1;
        }
		
		//for player1 purpose
        void Shoot()
        {
			//Get me an Inactive bullet to reuse.
			for(int i = 0 ; i < OBJECT_POOL_SIZE ;++i)
				if(bulletObjectPool[i]->GetActive()==false)
				{
					bulletObjectPool[i]->SetActive(true);
					bulletObjectPool[i]->SetX(m_X);
					bulletObjectPool[i]->SetY(m_Y-1);
					return ;
				}
					
			//Return the last one, which is rarely to be use.
			bulletObjectPool[OBJECT_POOL_SIZE-1];
			bulletObjectPool[OBJECT_POOL_SIZE]->SetActive(true);
			bulletObjectPool[OBJECT_POOL_SIZE]->SetX(m_X);
			bulletObjectPool[OBJECT_POOL_SIZE]->SetY(m_Y-1);
			return; 
        }
		
		//For player 2 purpose
		void Shoot(int X)
		{
			//Get me an Inactive bullet to reuse.
			for(int i = 0 ; i < OBJECT_POOL_SIZE ;++i)
				if(bulletObjectPool[i]->GetActive()==false)
				{
					bulletObjectPool[i]->SetActive(true);
					bulletObjectPool[i]->SetX(X);
					bulletObjectPool[i]->SetY(m_Y+1);
					return ;
				}
					
			//Return the last one, which is rarely to be use.
			bulletObjectPool[OBJECT_POOL_SIZE-1];
			bulletObjectPool[OBJECT_POOL_SIZE]->SetActive(true);
			bulletObjectPool[OBJECT_POOL_SIZE]->SetX(X);
			bulletObjectPool[OBJECT_POOL_SIZE]->SetY(m_Y+1);
			return; 
			
		}
        
        Bullet* GetBullet(int index)
        {
            return bulletObjectPool[index];
        }
	
};

//-------------------------------------------------------
//  Global
//-------------------------------------------------------

Spaceship Player1;
Spaceship Player2;
int loop=1;

//-------------------------------------------------------
//  Forward Declare Functions
//-------------------------------------------------------
void *thread_listenInput(void *arg);
void *thread_listenData(void *arg);

//-------------------------------------------------------
//  Other Functions
//-------------------------------------------------------

void InitializePlayers(Spaceship* s1, Spaceship* s2)
{	
    s1->SetID("Me");
	s1->SetX(XMAX/2);
	s1->SetY(YMAX-2);
    s2->SetID("Opponent");
	s2->SetX(XMAX/2);
	s2->SetY(1);
}

bool ChkCoord(int x, int y, int playerX, int playerY)
{
	if( y == playerY && x == playerX)
		return true;
	else
		return false;
}

bool ChkActiveBullet( int  x , int y , Spaceship *s)
{
	//If there are bullet are active, return true.
	bool result = false;
	for( int i = 0 ; i < OBJECT_POOL_SIZE ; ++i)
	{
		if(s->GetBullet(i)->GetActive() == true )
	    {
			int bX = s->GetBullet(i)->GetX();
			int bY = s->GetBullet(i)->GetY();
			result = result || ChkCoord( x, y, bX, bY);
	    }
	}
	
	return result;
}

bool ChkBulletCollision(Bullet *b1, Bullet* b2) 
{
	//Get Active Bullets from Player1 and Player2.
	
	// Exact Spot
	bool condition1 = b1->GetX() == b2->GetX() && b1->GetY() == b2->GetY() ;
	// Next to each other
	bool condition2 = b1->GetX() == b2->GetX() && b1->GetY()+1 == b2->GetY() ;
	return condition1 || condition2;
}

bool ChkBulletHitPlayer(Bullet *b)
{
	bool hitX_range = b->GetX() >= Player1.GetX() -2 && b->GetX() <= Player1.GetX() +2; 
	bool hitY_range = b->GetY() == Player1.GetY() || b->GetY() == Player1.GetY()-1;
	return hitX_range && hitY_range;
	
}

void UpdateLogics()
{
	//Player2 Position Update
	Player1.CapValue( 2, XMAX-3, 0, YMAX);
	Player2.CapValue( 2, XMAX-3, 0, YMAX);
	
	//Bullet Collision
	for( int i = 0 ; i < OBJECT_POOL_SIZE; ++i)
	{
		if(Player1.GetBullet(i)->GetActive() == true)
		{
			for(int j = 0; j < OBJECT_POOL_SIZE;++j)
			{
				if( Player2.GetBullet(i) -> GetActive() == true)
				{
					if(ChkBulletCollision( Player1.GetBullet(i) , Player2.GetBullet(j) ))
					{
						Player1.GetBullet(i)->SetActive(false);
						Player2.GetBullet(i)->SetActive(false);
					}
				}
			}	
		}
	}
		
	//Player Collision Check
	for(int i = 0 ; i < OBJECT_POOL_SIZE ;++i)
	{
		if(Player2.GetBullet(i)->GetActive() == true)
		{
			if(ChkBulletHitPlayer( Player2.GetBullet(i) ))
			{
				int value = 404;
				send(hSocket, &value, sizeof(int32_t), 0);
				Player1.SetAliveFalse();
			}
		}
	}
	
	//Bullets, Player1
    for(int i = 0 ; i < OBJECT_POOL_SIZE; ++i)
    {
		 if(Player1.GetBullet(i)->GetY() <= 1)
		 {
			Player1.GetBullet(i)->SetActive(false);
			Player1.GetBullet(i)->SetY(-1);
		 }
		if(Player1.GetBullet(i)->GetActive() == true)
			Player1.GetBullet(i)->SetY( Player1.GetBullet(i)->GetY() -1);

    }      
	
	//Bullets, Player2
	for(int i = 0 ; i < OBJECT_POOL_SIZE; ++i)
    {
		 if(Player2.GetBullet(i)->GetY() > YMAX-3)
		 {
			Player2.GetBullet(i)->SetActive(false);
			Player2.GetBullet(i)->SetY(-1);
		 }
		if(Player2.GetBullet(i)->GetActive() == true)
			Player2.GetBullet(i)->SetY( Player2.GetBullet(i)->GetY() +1);

    }      
}

void Draw()
{

	bool mainbody;
	bool left1;
	bool left2;
	bool right1;
	bool right2;
	bool A;
	bool V;	
	bool blanks;
	const int p1X = Player1.GetX();
    const int p1Y = Player1.GetY();
    const int p2X = Player2.GetX();
    const int p2Y = Player2.GetY();

 	system("clear");
	for ( int col = 0; col < YMAX; col++)
	{ 
		for ( int row = 0; row < XMAX; row++)
		{
			mainbody =  ChkCoord(row ,col,p1X, p1Y) || ChkCoord(row, col, p2X  , p2Y);
			left1 = ChkCoord(row, col, p1X -1, p1Y) || ChkCoord(row, col, p2X+1, p2Y);
			left2 = ChkCoord(row, col, p1X -2 ,p1Y) || ChkCoord(row, col, p2X+2, p2Y);
			right1 = ChkCoord(row, col, p1X+1, p1Y) || ChkCoord(row, col, p2X-1, p2Y);
			right2 = ChkCoord(row, col, p1X+2, p1Y) || ChkCoord(row, col, p2X-2, p2Y);
			A = ChkCoord (row, col, p1X, p1Y-1) || ChkActiveBullet(row, col, &Player1);
			V = ChkCoord (row, col, p2X, p2Y+1) || ChkActiveBullet(row, col, &Player2);
            blanks  =  (row > 0) && ( row < XMAX-1) && (col > 0) && (col < YMAX-1);
			
			if(mainbody)
				printf("H");
			else if(left1 || right1)
			    printf("-");
			else if(left2 || right2)
			    printf("+");
			else if(A)
			    printf("A");
			else if(V)
			    printf("V");
			else if (blanks)
				printf(" ");
			else 
				printf("#");
		}
	putchar('\n');
	}
}

int getch(void) 
{ 
    int c=0; 

    struct termios org_opts, new_opts; 
    int res=0; 
      //-----  store old settings ----------- 
    res=tcgetattr(STDIN_FILENO, &org_opts); 
    assert(res==0); 
      //---- set new terminal parms -------- 
    memcpy(&new_opts, &org_opts, sizeof(new_opts)); 
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL); 
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts); 
    c=getchar(); 
      //------  restore old settings --------- 
    res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts); 
    assert(res==0);
    return(c); 
}

int main(int argc, char *argv[] )
{ 
	if(!PrompServerOrClient(argc,argv))
        return -1;
	
    if(!MakingSocket() )
		return -1;
		
	if (!EstablishConnection())
		return -1;
		
	
    /*if( !Connected())
		return -1;*/

    //Game Objects Initialize
    InitializePlayers(&Player1, &Player2);
	
    // Thread Initialize
    int i, tmp;
    int arg[NUM_THREADS] = {0,0};

    pthread_t thread[NUM_THREADS];
    pthread_attr_t attr;

    //initialize and set the thread attributes
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
 
    //creating threads 
    for ( i=0; i<NUM_THREADS; i++ )
    { 
		if( i == 0)
			tmp = pthread_create( &thread[i], &attr, thread_listenInput,  (void *)&arg[i] );
		
		if( i ==1 )
			tmp = pthread_create(&thread[i], &attr, thread_listenData, (void *)&arg[i]);
		
        if ( tmp != 0 )
        { 
            fprintf(stderr,"Creating thread %d failed!i\n",i);
             return 1;
        }
    }
    
	
    // Main game Loop
    while(loop)
    { 
		//Cap Value on Horizontal Movement
		UpdateLogics();
        Draw();
		if(Player1.GetAliveState() == false || Player2.GetAliveState() == false )
			break;
        usleep(100000);
    }
	
	if(Player1.GetAliveState() == false)
		printf("Player2 Win! You Lose!");
	
	if(Player2.GetAliveState() == false)
		printf("You Win! Player2 Lose!");

    //joining threads
    for ( i=0; i<NUM_THREADS; i++ )
    { 
        tmp = pthread_join( thread[i], NULL );
        if ( tmp != 0 )
        { 
            fprintf(stderr,"Joining thread %d failed!\n",i);
            return 1;
        }
    }

}

//-------------------------------------------------------
//  Threading Functions
//-------------------------------------------------------

void *thread_listenInput( void *arg)
{ 
    int posX = -1;
    int flags = fcntl(hSocket, F_SETFL, flags | O_NONBLOCK) ;

    while(loop!='q')
    {
        loop = getch();
        switch (loop)
        { 
            case 'a':
                Player1.SetX(Player1.GetX() -1 ) ; 
				posX = XMAX - Player1.GetX() +256; 
				send(hSocket, &posX, sizeof(int32_t),0 );
				posX = -1;
				break;
            case 'd': 
                Player1.SetX(Player1.GetX() +1 ) ;
				posX = XMAX - Player1.GetX() +256; 
				send(hSocket, &posX, sizeof(int32_t),0 );
				posX = -1;
				break;
            case 'w':
                Player1.Shoot(); 
				posX = XMAX - Player1.GetX() ;
				send(hSocket, &posX, sizeof(int32_t),0 );
				posX = -1;
				break;
            case 'q': 
                loop = 0;
        }
    }
    pthread_exit( NULL );
}

void *thread_listenData( void* arg)
{

	int value = -1;
	int flags =  fcntl(hSocket, F_SETFL,  O_NONBLOCK) ;
	while(1)
	{
		//Set 0 for both side, if i use flag, it is not working.
		recv( hSocket, &value, sizeof(int32_t), 0);
			
		if(value >= 0)
		{
			if( value == 404)
			{
				Player2.SetAliveFalse();
			}
			
			//Player2's position
			if(value > 256)
			{
				p2PosX = value-256;
				Player2.SetX(p2PosX);
			}
	
			//Player2's bullets
			if (value >= 0 && value < XMAX)
			{
				bulletX = value;
				Player2.Shoot(bulletX);
			}
		}	
	
		//reset the value
		value = -1;
		
    }
    pthread_exit( NULL );
}

