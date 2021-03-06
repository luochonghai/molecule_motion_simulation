#include"stdafx.h"
#include"Fly.h"
#include"calculation_tool.h"
using namespace std;

int main()
{
	long long ColWall[3],ColNode = 0, timer = 0;//used to count the collapse time
	ColWall[0] = ColWall[1] = ColWall[2] = 0;
	timer = 0;//to count how many periods have passed
	//generate random num
	srand(unsigned(time(NULL)));
	//here the following line is a must when using "normal_distribution" to generate random num
	default_random_engine gener;
	normal_distribution<double> norm_v_len{V_BAR,V_STA_DIV};
	Fly *groups = new Fly[SIZE];
	int ColliSig[SIZE];//to record whether nodes collide or not in each time step
	for (int i = 0; i < SIZE; ++i)
	{
		double initial_cor[3], initial_vel[3],ini_v;

		//to initialize the coordinate of fly
		for (int m = 0; m < 3; ++m)
			initial_cor[m] = (rand() % (2 * int(ColHalfScale - LEN_FLY))) - ColHalfScale + LEN_FLY;

		//to initialize the velocity of fly
		do
		{
			ini_v = norm_v_len(gener);
		} while (ini_v < V_BAR-V_STA_DIV || ini_v > V_BAR+V_STA_DIV);
		for (int m = 0; m < 3; ++m)
		{
			//the velocity of each fly cannot exceed the maximal speed
			initial_vel[m] = cos(rand()%360);
		}
		groups[i] = Fly(initial_cor,initial_vel,ini_v);
	}

	system("DEL /f D:\\FDU\\NeuroScience\\1904FruitflyExp\\coor_cpp.txt");
	cout << "The last-time-originated data has been deleted!" << endl;
	ofstream coordi;
	coordi.open("D:\\FDU\\NeuroScience\\1904FruitflyExp\\coor_cpp.txt",ios::out|ios::app);

	//now start simulation
	for (timer = 0; timer*DELTA_T < LIFE; timer++)//time cycle(1st)
	{
		if (!timer)
		{
			for (int r = 0; r < SIZE; ++r)
			{
				coordi << r << " ";//1st: write the index of fly into .txt
				for (int u = 0; u < 3; ++u)
				{
					auto cor_num = groups[r].original_state()[u];
					coordi << " " << cor_num << " ";//2nd :write (x,y,z) into .txt
				}
				coordi << endl;
			}
		}

		double group_flys[SIZE][3];//the current coordinate of fly
		for (int c_i = 0; c_i < SIZE; ++c_i)
		{
			auto temp_cor = groups[c_i].original_state();
			for (int c_j = 0; c_j < 3; ++c_j)
			{
				group_flys[c_i][c_j] = temp_cor[c_j];
			}
		}

		//to initialize ColliSig array
		/*  0: initialize
		   -1: meet wall
		  >=1: collide with other nodes for times*/
		for (int j = 0; j < SIZE; ++j)
			ColliSig[j] = 0;

		//to check whether wall_node collision happens
		for (int j = 0; j < SIZE; ++j)
		{
			/*here we suppose no more than one collision happens in a time step.*/
			ColliSig[j] = groups[j].pinball();
			if (ColliSig[j] > 0)
			{
				//cout << "Col: " << ColliSig[j] << endl;
				ColWall[ColliSig[j]-1]++;
				ColliSig[j] = -1;
				continue;
			}
		}

		//to check whether node_node collision happens
		for (int j = 0; j < SIZE; ++j)
		{
			if (ColliSig[j] < 0)//have collided with wall
				continue;
			//when wall-node collision not happen
			for (int k = j+1; k < SIZE; ++k)
			{
				if (ColliSig[k] < 0)//have collided with wall
					continue;
				//change_velocity_direction when 2 nodes collide for the first time(2nd)
				double k_vel[3], k_coo[3];//to record the modified vel and coordiante of node k
				for (int vi = 0; vi < 3; ++vi)
				{
					k_vel[vi] = groups[k].original_vel()[vi];
					k_coo[vi] = groups[k].original_state()[vi];
				}
				bool sig_col = groups[j].transfer(k_vel, k_coo,groups[k].v_len);//to check whether node j and k collide with each other
				if (sig_col == true)//collision happens
				{
					++ColNode, ++ColliSig[j], ++ColliSig[k];
					//modify node k's next_vel and next_cor
					groups[k].ModAnoVel(k_vel,k_coo);
				}
			}
		}

		for(int j = 0;j < SIZE;++j)//change_coordinate(4th)
		{
			if (ColliSig[j] == 0)//collision not happen
				groups[j].transfer();//let node go straight ahead
			else
				groups[j].updateCol();//update the node's coordinate
			coordi << j << " ";
			for (int u = 0; u < 3; ++u)
			{
				auto temp_num = groups[j].original_state()[u];
				if (abs(temp_num) > ColHalfScale)
					cout << "There's sth. wrong with my code... " <<temp_num<< endl;
				coordi  << " " << temp_num << " ";
			}
			coordi << endl;
		}
	}
	delete []groups;
	coordi.close();
	cout << "Collide with wall:" << endl;
	for (int ci = 0; ci < 3; ++ci)
		cout << ci + 1 << " times: " << ColWall[ci] << endl;
	cout << "Collide with nodes: " << ColNode << endl;
    return 0;
}
