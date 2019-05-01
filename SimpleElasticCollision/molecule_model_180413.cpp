#include"stdafx.h"
#include"Fly.h"
#include"calculation_tool.h"
using namespace std;
using namespace cv;

static int timer;//used to control the system_operating_time
static int ColWall,ColNode;//used to count the collapse time

int main()
{
	ColWall = ColNode = 0;
	timer = 0;//to count how many periods have passed
	//generate random num
	srand(unsigned(time(NULL)));
	//here the following line is a must when using "normal_distribution" to generate random num
	default_random_engine gener;
	normal_distribution<double> norm{V_BAR,V_STA_DIV};
	Fly *groups = new Fly[SIZE];
	bool ColliSig[SIZE];//to record whether nodes collide or not in each time step
	for (int i = 0; i < SIZE; ++i)
	{
		double initial_cor[3], initial_vel[3];
		for (int m = 0; m < 3; ++m)
		{
			//to initialize the coordinate of fly
			initial_cor[m] = (rand() % (2 * int(ColHalfScale-LEN_FLY))) - ColHalfScale+LEN_FLY;
			do 
			{
				//the velocity of each fly cannot exceed the maximal speed
				initial_vel[m] = norm(gener);
			} while (abs(initial_vel[m]) > 2 * V_BAR);
		}
		groups[i] = Fly(initial_cor,initial_vel);
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
		for (int j = 0; j < SIZE; ++j)
			ColliSig[j] = false;

		for (int j = 0; j < SIZE; ++j)
		{
			//change_velocity_direction when a node meet the walls(3rd)
			/*here we suppose no more than one collision happens in a time step.*/
			if (ColliSig[j] == false)
				ColliSig[j] = groups[j].pinball();
			else
				continue;
			if (ColliSig[j])//when wall-node collision happens
			{
				ColWall++;
				continue;
			}

			//when wall-node collision not happen
			for (int k = 0; k < SIZE; ++k)
			{
				if (k != j)
				{
					//change_velocity_direction when 2 nodes collide(2nd)
					double k_vel[3];//to record the modified vel of node k
					for (int vi = 0; vi < 3; ++vi)
						k_vel[vi] = groups[k].original_vel()[vi];
					ColliSig[j] = groups[j].transfer(k_vel,groups[k].original_state());
					if (ColliSig[j])
					{
						ColNode++;
						if (k > j)
						{
							ColliSig[k] = true;//this node has collided as well
							groups[k].ModAnoVel(k_vel);//modify k node's vel
						}
						break;
					}
				}
			}
		}

		for(int j = 0;j < SIZE;++j)//change_coordinate(4th)
		{
			if (ColliSig[j] == false)//collision not happen
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

	cout << "Collide with wall: " << ColWall << endl;
	cout << "Collide with nodes: " << ColNode << endl;
    return 0;
}

//int test_main()
//{
//	double A[3] = {1,0,1}, 
//		B[3] = {0,0,0}, 
//		C[3] = {-1,0,-1}, 
//		NormalVec[3] = {0,0,1}, D[3], wall_val = 0;
//	RefSolu(B, C, NormalVec, D);
//	for (int i = 0; i < 3; ++i)
//		cout << D[i] << endl;
//}