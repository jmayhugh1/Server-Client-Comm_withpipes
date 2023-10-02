/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:
	UIN:
	Date:
*/
#include "functions.cpp"
#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>

using namespace std;
int MAX_MESSAG = MAX_MESSAGE;

int main (int argc, char *argv[]) {
	int opt;
	int p = -1; //patient number
	double t = -1; //time in seconds
	int e = -1; //ecg number
	int m = MAX_MESSAGE;
	bool newc = false;
	vector<FIFORequestChannel*> channels;
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) //colon is used to identify value
	{
		switch (opt) {
			case 'p':
				p = atoi (optarg);

				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi (optarg);
				break;
			case 'c':
				newc = true;
				break;

		}
	}
	
	// give arguments for the server
	// server needs './server', '-m', '<val for -marg>', 'NULL'
	
	
	// fork
	
	// in the child rin execvp using the server arguments


    
	//create a new process for the server
	// fork
	pid_t server_process_id = fork();
	if (server_process_id < 0)
	{
		//error
		cout << "Error in fork" << endl;
		return 1;
	}

	// execute the server code
	if (server_process_id == 0) // if it is the child
	{
		//CHILD
		//create an array that contains the executable and the arguments
		char * args [] = {const_cast<char *> ("./server"), const_cast<char *>("-m"), const_cast<char *>(to_string(m).c_str()), nullptr};
		int ret = -1;
		ret = execvp(args[0], args);
		if (ret < 0)
		{
			//error
			cout << "Error in execvp" << endl;
		}

	}
	
	//vector of class channely type
	
	FIFORequestChannel control_chan("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&control_chan);
	
	
	
	if (newc){ //send new channel request to the server
		MESSAGE_TYPE nm = NEWCHANNEL_MSG;
		control_chan.cwrite(&nm, sizeof(MESSAGE_TYPE));
		char newchanname[100];
		control_chan.cread(newchanname, sizeof(newchanname));

		FIFORequestChannel* newchan = new FIFORequestChannel(newchanname, FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(newchan);


	}
	FIFORequestChannel chan = *(channels.back());
	
	if (p != -1 and t != -1 and e != -1)
	{
		// example data point request
		char buf[MAX_MESSAGE]; // 256
		datamsg x(p, t, e); //changr from hard coded to the user input
		std::cout << "Size of datamsg: " << sizeof(datamsg) << std::endl;
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	else if(p != -1){ //request 1000 datapoints
		//loop over 1st 1000 lines
		ofstream myfile;
		myfile.open("./received/x1.csv");
		//checl if file is open
		if (!myfile.is_open())
		{
			cout << "Error opening file" << endl;
			return 1;
		}
		for (int i = 0; i < 1000; i++)
		{
			double time = i * 0.004;
			double ecg1 = 0;
			double ecg2 = 0;
			//send request for ecg 
			char buf[MAX_MESSAGE]; // 256
			datamsg x(p, time, 1); //changr from hard coded to the user input
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			ecg1 = reply;

			//send request for ecg 2
			char buf2[MAX_MESSAGE]; // 256
			datamsg x2(p, time, 2); //changr from hard coded to the user input
			memcpy(buf2, &x2, sizeof(datamsg));
			chan.cwrite(buf2, sizeof(datamsg)); // question
			double reply2;
			chan.cread(&reply2, sizeof(double)); //answer
			ecg2 = reply2;

			//write line to recieved/x1.csv
			
			
			myfile << time << "," << ecg1 << "," << ecg2 << endl;
			
		}
		myfile.close();
		
	}
	else if (filename != "") //-m is optional buffer capacity
	{
	//split up file into segments
	// if file is longer than buffercap it must be split into segments
    // sending a non-sense message, you need to change this
	//request buffer is size of file maessafe + filename
	//response buffer is size of buffer capacity
	
	filemsg fm(0, 0);
	string fname = filename;
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;
	int64_t filesize = 0;
	chan.cread(&filesize, sizeof(int64_t));
	//create a buffer od size buffer capacity (m)

	char* buf3 = new char[m];
	cout << "File length is: " << filesize << endl;
	int64_t remaining_bytes = filesize;
	int tempm = m;
	ofstream myfile;
	myfile.open("./received/" + filename, ios::out | ios::app | ios::binary);
		//check if file is open
		if (!myfile.is_open())
		{
			cout << "Error opening file" << endl;
			return 1;
		}
	// loop over the segments in the file filesize/buff capacity(m)
	while (remaining_bytes > 0){
		m = tempm;
		if (remaining_bytes < m)
		{
			m = remaining_bytes;
		}
		int64_t offse = filesize - remaining_bytes;
		
		// create filemsg instance
		filemsg* file_req = (filemsg*) buf2;
		file_req->offset = offse;
		file_req->length = m;//min(m, len);
		///cout << "Remaining bytes: " << remaining_bytes << endl;
		//cout << "Offset: " << file_req->offset << endl;
		//cout << "Length: " << file_req->length << endl;
		
		//senf the request (buf2)
		chan.cwrite(buf2, len);
		
		// recieve the response
		//cread into buf3 length file_reg->len
		chan.cread(buf3, file_req->length);
		//write buf3 to the file
		//open the file
		
		
		//write to the file
		myfile.write(buf3, file_req->length);
		
		remaining_bytes -= m;
	}
	myfile.close();
	delete[] buf2;
	delete[] buf3;
	}

	/*
	//how to transfer the file from the server ?
	// filesize = len
	
	// ...start timer
	
	remaining_bytes = filesize;

	while(remaininf bytes is not 0){
		if the remainng bytes is less than the bufer size
			lenght = remaining bytes
		offset = filesize - remaining bytes
		make a file transfer
		filemsg
	}
	*/
	//close and delete the channels
	if (newc){
			MESSAGE_TYPE mm2 = QUIT_MSG;
			channels[1]->cwrite(&mm2, sizeof(MESSAGE_TYPE));
			delete channels[1];
	}
		
		
	
	// closing the channel    
    MESSAGE_TYPE mm = QUIT_MSG;
    control_chan.cwrite(&mm, sizeof(MESSAGE_TYPE));
	wait(NULL);
	return 0;
}
