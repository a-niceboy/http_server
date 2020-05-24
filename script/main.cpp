#include "typedefine.h"
#include "Server.h"

InterMediary g_IM;

int G_Child_PID;
string G_HOST;

void sig_handler_parent(int sig)
{
	switch(sig) 
	{
		case SIGTERM:
		case SIGINT:
			if(G_Child_PID > 0)
			{
				kill(G_Child_PID, sig);
			}
			wait(0);
			exit(0);
			break;
	}
}

void sig_handler_child(int sig)
{
	switch(sig) 
	{
		case SIGTERM:
		case SIGINT:
			exit(0);
			break;
	}
}

int g_single_proc_inst_lock_fd = -1;

void single_proc_inst_lockfile_cleanup()
{
    if(g_single_proc_inst_lock_fd != -1)
    {
        close(g_single_proc_inst_lock_fd);
        g_single_proc_inst_lock_fd = -1;
    }
}

bool is_single_proc_inst_running(const char *process_name)
{
    char lock_file[128];
    snprintf(lock_file, sizeof(lock_file), "/var/tmp/%s.lock", process_name);

    g_single_proc_inst_lock_fd = open(lock_file, O_CREAT | O_RDWR, 0644);
    if(-1 == g_single_proc_inst_lock_fd)
    {
        fprintf(stderr, "Fail to open lock file(%s). Error: %s\n", lock_file, strerror_s(errno).c_str());
        return false;
    }

    if(0 == flock(g_single_proc_inst_lock_fd, LOCK_EX | LOCK_NB))
    {
        atexit(single_proc_inst_lockfile_cleanup);
        return true;
    }

    close(g_single_proc_inst_lock_fd);
    g_single_proc_inst_lock_fd = -1;
    return false;
}

int main_process(int argc, char *argv[])
{
	if(argc != 2)
	{
	    cout << "usage:" << endl;
	    cout << "exe + ThreadNumber" << endl;
	    
	    return -1;	
	}
	
	srand(time(NULL));
	
    string sIp = getHostIp();
    cout << "sIp is: " << sIp << endl;
    
    G_HOST = "http://" + sIp + ":" + lexical_cast<string>(PORT) + "/";
    cout << "G_HOST is: " << G_HOST << endl;

    HttpServer server(ServerConfig(PORT, 128, atoi(argv[1])));
    server.doJob();

    return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
	    cout << "usage:" << endl;
	    cout << "exe + ThreadNumber" << endl;
	    
	    return -1;	
	}
    
    string sExeName = getSelfName();
	cout << sExeName << endl;
    
	if(is_single_proc_inst_running(sExeName.c_str()) == false)
	{
		cout << sExeName << " is run. exit..." << endl;
		return -2;
	}
    	
	signal(SIGTERM, sig_handler_parent);
	signal(SIGINT,  sig_handler_parent);
	
	while(true)
	{
		G_Child_PID = fork();
		if(G_Child_PID < 0)
		{
			fprintf(stderr, "fork error ret = %d\n", G_Child_PID);
			exit(-1);
		}
		else if(G_Child_PID == 0) //child
		{
			main_process(argc, argv);
		}
		else
		{
			cout << "waiting for [pid = " << G_Child_PID << "}" << endl;
			waitpid(G_Child_PID, 0, 0);
			cout << "waiting ends" << endl << endl;
		}
		
		sleep(1);
	}
	
	return 0;
}
