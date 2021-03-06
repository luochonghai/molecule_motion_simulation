#include"stdafx.h"
#include"Fly.h"
#include"Interaction.h"
#include"calculation_tool.h"
using namespace std;

static int timer;//used to control the system_operating_time
static int collapse_time;//used to count the collapse time

void count_collapse(double group[SIZE][3])
{
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			double dis_square = 0;
			for (int k = 0; k < 3; ++k)
			{
				double cor_minus = group[i][k] - group[j][k];
				dis_square += cor_minus * cor_minus;
			}
			if (j != i && pow(dis_square, 0.5) < C_FIELD_TIME*LEN_FLY)
				collapse_time++;
		}
	}
}

int main()
{
	Py_Initialize();
	//to initialize the py_interaction_mode
	Interaction inter_py(SIZE,DELTA_T,1,1,HALF_SCALE,LIFE);
	inter_py.Py_evaluation_expdata("random_index_cal","cpp_invoke");

	timer = 0;
	srand(unsigned(time(NULL)));
	Fly *groups = new Fly[SIZE];
	for (int i = 0; i < SIZE; ++i)
	{
		double initial_cor[3];
		for (int m = 0; m < 3; ++m)
		{
			initial_cor[m] = (rand() % (2 * HALF_SCALE)) - HALF_SCALE;
		}
		groups[i] = Fly(rand()%2,(rand()%SIZE)/SIZE+1, initial_cor);
	}

	system("DEL /f D:\\FDU\\Template\\CS\\数学建模\\coordinate.txt");
	cout << "The last-time-originated data has been deleted!" << endl;
	ofstream coordi;
	coordi.open("D:\\FDU\\Template\\CS\\数学建模\\coordinate.txt",ios::out|ios::app);
	for (timer = 0; timer*DELTA_T < LIFE; ++timer)//time cycle(1st)
	{
		if (!timer)
		{
			for (int r = 0; r < SIZE; ++r)
			{
				coordi << r << " ";
				for (int u = 0; u < 3; ++u)
				{
					auto cor_num = groups[r].original_state()[u];
					coordi << " " << cor_num << " ";
				}
				coordi << 0 << " ";
				coordi << endl;
			}
		}

		double group_flys[SIZE][3];
		for (int c_i = 0; c_i < SIZE; ++c_i)
		{
			auto temp_cor = groups[c_i].original_state();
			for (int c_j = 0; c_j < 3; ++c_j)
			{
				group_flys[c_i][c_j] = temp_cor[c_j];
			}
		}
		if (timer*DELTA_T > STABLE_RATIO*LIFE)//to count how many times the flies would collapse;since the initial state would be made up of NaN data
		{
			count_collapse(group_flys);
		}

		for (int j = 0; j < SIZE; ++j)//change_force_cycle(2nd)
		{
			groups[j].refresh_A();//each time initialize fly-a's a to be zero
			for (int k = 0; k < SIZE; ++k)
			{
				if (k != j)
				{
					groups[j].transfer(groups[k].original_state(), timer);//to change the a of fly-j because of fly-k
				}
			}
		}
		for(int j = 0;j < SIZE;++j)//change_coordinate&speed_cycle(3rd)
		{
			groups[j].transfer();//now calculate the coordinate of fly-j
			coordi << j << " ";
			for (int u = 0; u < 3; ++u)
			{
				auto temp_num = groups[j].original_state()[u];
				coordi  << " " << temp_num << " ";
			}
			coordi << groups[j].show_cur_radius() << " ";
			coordi << endl;
		}
	}
	delete []groups;
	coordi.close();

	inter_py.Py_evaluation_expdata("read_cordinate_text", "model_evaluation");
	Py_Finalize();
	cout << collapse_time << endl;
    return 0;
}