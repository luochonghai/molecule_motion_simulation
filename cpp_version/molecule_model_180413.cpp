#include"stdafx.h"
#include"calculation_tool.h"

/*NOTICE: the system_parameters are set in SI units by default. 
Ohterwise it would be declared.*/
#define HALF_SCALE 30//cm
#define SIZE 100
#define DELTA_T 0.1
#define PARA_2 40.0
#define PARA_3 0.025
#define PARA_4 3.0//the order of force(mainly used to control the fast&active the system is operating)
#define LIFE 1000
#define STABLE_RATIO 0.5//the ratio is used to count when to start calculating the collapse of flies
#define LEN_FLY 5/10//cm
#define R_FIELD_LEN 400000 //receptive field radius(cm)
#define C_FIELD_TIME 3  //colliding field radius
//right now the solution of Force(x) = 0 is 0.129, we regard it as the 5 times of length of fly
#define sqrt2 sqrt(2.0)
#define ts_cos_theta -1.0/1

using namespace std;

static int timer;//used to control the system_operating_time
static int collapse_time;//used to count the collapse time

class Fly
{
public:
	Fly() = default;
	Fly (bool sex_fly,double weight_fly,double initial_cor[3]) :
		sex(sex_fly),m(weight_fly),cor(),vel(),acv(),cur_radius(0)
	{
		for (int i = 0; i < 3; ++i)
			rec_fie_cen[i] = cor[i] = initial_cor[i];
	}
	double* original_state()//print the coordinate of node
	{
		return cor;
	}

	double show_cur_radius()
	{
		return cur_radius;
	}

	void refresh_A()//to refresh the accelerated velocity of node to be zero
	{
		for (int i = 0; i < 3; ++i)
		{
			acv[i] = 0;
		}
	}
	void transfer(double another_node[3],int timer)//to change the accelerated velocity of Fly
	{
		transfer_A(another_node,timer);
	}
	double* transfer()//when the accelerated velocity has been calculated, calculate the new coordinate
	{
		return transfer_state();
	}

	//to return the coordinate of the fly which is used to calculate whether it would collapse with others
	double *reception_field_collapse()
	{
		double R = C_FIELD_TIME * LEN_FLY / sqrt2;
		double *res = new double[3];
		double v_len = sqrt(v2_multi(vel,vel,3));
		for (int i = 0; i < 3; ++i)
		{
			res[i] = cor[i]+R*vel[i] / v_len;
		}
		return res;
	}
private:
	bool sex;
	//the fly's weight
	double m;
	//the fly's coordinate(x,y,z)
	double cor[3];
	//the fly's speed
	double vel[3];
	//the fly's accelerated velocity
	double acv[3];
	//the fly's radius of curvature
	double cur_radius;
	//center of the reception field
	double rec_fie_cen[3];

	/*180610: now we add reception field to the model, so it's necessary to check whether fly-k 
	can influence fly-j or not first. We use cos(Vj,Vk)&Radius(Reception)-D(i,j) to check it.
	Then it seemed to be wrong since the flies cannot cluster again because no fly would 
	drive them back to the central point when they dissipate.
	So I took another method: use a ball to depict the reception field of the fly,while the center
	isn't the fly's coordiante--instead, both the fly's coordinate and velocity_direction determine
	the centre of the ball.*/
	void transfer_A(double another_node[3],int timer)
	{
		double delta_cor[3], force[3];
		for (int i = 0; i < 3; ++i)
		{
			delta_cor[i] = cor[i] - another_node[i];
		}
		double distance = sqrt(v2_multi(delta_cor,delta_cor,3));

		//check first
		/*one way to check:
		if (distance > R_FIELD_LEN)//condition 2 isn't satisfied
			return;
		else
		{
			double vel_len = sqrt(v2_multi(vel,vel,3));
			double cos_vec_multi = -v2_cos(vel,delta_cor,3);
			if (cos_vec_multi <= ts_cos_theta)
				return;
		}
		*/

		/*another way to check:change the reception field*/
		double delta_dis[3];
		for (int di = 0; di < 3; ++di)
			delta_dis[di] = another_node[di] - rec_fie_cen[di];
		if (sqrt(v2_multi(delta_dis, delta_dis, 3)) > R_FIELD_LEN)
			return;

		double force_pair = PARA_2/pow(distance,3)-PARA_3/distance;
		force_pair *= PARA_4;
		
		/*I don't know what's wrong with my code, but the simulation of molecule seems wrong!
		Now I got the bug:
		1.the system bug:
		I calculated the coordinate of each modecule before a round is over, which would 
		influence the remaining modecules' coordinate to be wrong.
		2.the parameter-set bug:
		Since the calculating ability of computer,especially our PC, is limited, 
		When the distance of two modecules is very small, and the force function
		is quite accurate,such as 1/s^5, then you would fall into cases when the 2
		modecules are quite close, their repulsive force would be extremely large
		with errors we cannot ignore!*/
		for (int j = 0; j < 3; ++j)
		{
			force[j] = force_pair * delta_cor[j]/distance;
			acv[j] += force[j] / m;
		}

	}

	double *transfer_state()
	{
		double vec_dir[3];
		norm_vec(vel,vec_dir,3);
		for (int i = 0; i < 3; ++i)
		{
			rec_fie_cen[i] = cor[i] + R_FIELD_LEN / 2 *vec_dir[i] ;
			cor[i] += (vel[i] + acv[i] * DELTA_T / 2.0)*DELTA_T;
			vel[i] += acv[i] * DELTA_T;
		}

		double force[3];

		for (int j = 0; j < 3; ++j)
		{
			force[j] = acv[j] * m;
		}
		//to calculate the curvature radius and the velocity_radial / V via one point's coordinate
		double v_2norm = sqrt(v2_multi(vel,vel,3));
		double f_2norm = sqrt(v2_multi(force,force,3));
		double force_tangential = (v2_multi(force,vel,3) / v_2norm);
		double force_radial = sqrt(f_2norm*f_2norm - force_tangential * force_tangential);
		cur_radius = (m*v_2norm*v_2norm + 3 * force_tangential*v_2norm*DELTA_T) / force_radial;

		return cor;
	}
};

class Interaction
{
public:
	Interaction() = default;
	Interaction(int Sample_size,double Interval,double K_index,double Length_dro,double Length_box,double Terminal):
	sample_size(Sample_size),interval(Interval),K(K_index),length_dro(Length_dro),length_box(Length_box),terminal(Terminal){}
	void Py_evaluation_expdata(const char* file_name, const char* module_name)
	{
		py_evaluation(file_name,module_name);
	}
private:
	int sample_size;
	double interval;
	double K;
	double length_dro;
	double length_box;
	double terminal;
	bool py_evaluation(const char* file_name, const char* module_name)
	{
		//Py_Initialize();

		//to state the variable(s)
		PyObject *pModule = NULL;
		PyObject *pFunc = NULL;

		//IMPORTANT!!change the current_file_path
		PyRun_SimpleString("import sys;sys.path.append('./')");

		//define invoking file's name
		PyObject* moduleName = PyUnicode_FromString(file_name);
		pModule = PyImport_Import(moduleName);
		if (!pModule)
		{
			cout << "[ERROR]:Python get module failed." << endl;
			return 0;
		}
		cout << "[INFO]Python get module succeed." << endl;
		//pModule = PyImport_ImportModule("random_index_cal");


		//define the invoking function's name
		pFunc = PyObject_GetAttrString(pModule, module_name);

		//invoke the function
		//PyEval_CallObject(pFunc, NULL);

		//finish the invoking process
		//Py_Finalize();

		/*test ok*/
		//PyRun_SimpleString("import sys; sys.path.append('.')");
		//PyRun_SimpleString("import mytest;");
		//PyRun_SimpleString("print(mytest.myabs(-2.0))");
		//Py_Finalize();
		return 0;
	}
};

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
			if (pow(dis_square, 0.5) < C_FIELD_TIME*LEN_FLY)
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

	//to store the coordinate of simulation_model

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
			auto temp_cor = groups[c_i].reception_field_collapse();
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
	coordi.close();

	inter_py.Py_evaluation_expdata("read_cordinate_text", "model_evaluation");
	Py_Finalize();
	cout << collapse_time << endl;
    return 0;
}

