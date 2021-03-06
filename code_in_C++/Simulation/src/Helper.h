#ifndef HELPER_H
#define HELPER_H

#include "Grid.h"
#include "Specie.h"
#include "SimTypes.h"
#include "color.h"

#include <iostream>
#include <chrono>
#include <unordered_map>

#include <fenv.h>

#define N_INPUT 2


namespace SimEco{

	enum class Exceptions {fileException=1, decompStrException, memRealocException, argException, configurationException=5};
	
	int intException(enum Exceptions ex);
	void testArgs(int argc,char const *argv[]);

	void create_SimulationDirectory(const char *simName);
	void create_timeStepsDirs(const char *_name, int nSteps, vector<bool> &createdDir);

	void recordSimulationInfo(Simulation &simulation_src);

	void recordTimeStepFiles(const char *path, int timeStep, Grid g, const char *simName);

	void recordSpecieTxtFile(const char *path, int timeStep, Specie &sp);
	void recordSpecieFile(const char *path, int timeStep, Specie &sp);



	//Timer class
	class Clock
	{   
		private:
		typedef struct {
			std::chrono::high_resolution_clock::time_point startClock;
			std::chrono::high_resolution_clock::time_point pausedClock;
			bool isPaused;
		}Clock_count;

		Clock_count timer;

		public:
		//std::chrono::high_resolution_clock::time_point start;
		//std::chrono::high_resolution_clock::time_point end;
		//std::chrono::duration<double> elapsed;

		void Start();
		//pausa o timer
		void Pause();
		//continua o timer
		void Continue();
		//para o timer se nao estiver pausado já. E então retorna o tempo percorrido total pelo timer.
		double End();
		//double elapsedTime();
	};

	class Configuration
	{	public:
		static char NAME[65];
		static char SAVEPATH[265];
		static unsigned int MAX_CELLS;
		static unsigned int NUM_FOUNDERS;
		static unsigned int TIMESTEPS;
		static unsigned int YEARS_PER_TIMESTEP;

		static string MinTemp_dataSource;
		static string MaxTemp_dataSource;
		static string MinPPTN_dataSource;
		static string MaxPPTN_dataSource;
		static string NPP_dataSource;

		static string Lat_dataSource;
		static string Lon_dataSource;

		static string Areas_dataSource;

		static string Neighbors_dataSource;

		
		void Configure();
	};

}

#endif