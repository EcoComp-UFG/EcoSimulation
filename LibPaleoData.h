
namespace SimEco{

	class TPaleoClimate;

	// definindo o time temporariamente. Interpretei o nome do tipo no arquivo pascal para o mais proximo em C++
	typedef float** TSngMatrix;
	typedef float* TSngVector;
	typedef TSngVector* PSngVector;
	typedef double** TDblMatrix;
	typedef TPaleoClimate* PPaleoClimate;



	typedef struct{
		int prevTimeStep, nextTimeStep;
		float prevSATMin, prevSATMax;
		float prevPPTNMin, prevPPTNMax;
		float prevNPP;

		float nextSATMin, nextSATMax;
		float nextPPTNMin, nextPPTNMax;
		float nextNPP;
	}TLastInterpolation;


	double roundTo(double number, int precision);
	float roundTo(float number, int precision);


	class TPaleoClimate {

	  private:
		int modelGridnCells;
		TSngVector PLASIMLats, PLASIMLongs;
		TSngVector modelGridLat, modelGridLong;

	  	int PLASIMnTime;
	  	int PLASIMnTimeOffset;	// 1 if PLASIMnTime = 5001, and 10 if PLASIMnTime = 50001

	  	short *adjLeft, *adjDown;	// PLASIM grid cells relating to model grid

	  	TSngMatrix *PLASIMDataSATMax, *PLASIMDataSATMin;
	  	TSngMatrix *PLASIMDataPPTNMax, *PLASIMDataPPTNMin;
	  	TSngMatrix *PLASIMDataNPP;

	  	TSngVector wLon, wLat;		//Weights for interpolation

	  	TDblMatrix modelGridObsClimate;			// Baseline/current climate from "observed" data (e.g. WorldClim)
	  	TSngMatrix modelGridPLASIMClimate;		// Predictions of PLASIM for the current climate

	  	TLastInterpolation *lastInterpolation;
	  	bool projAnomalies;

	  public:
	/*construtor*/

	  	// Create the class by opening the PLASIM file and the present-day climatology data as baseline
	  	//LibPaleoData(String PLASIMFile, String presentClimateFile, bool projectAnnomalies);
	  	TPaleoClimate(char* PLASIMFile, char* presentClimateFile, bool projectAnnomalies);

	/****metodos****/

		/* Interpolate only one cell, and get the four climatic variables for the cell
			c-> Index of the model grid cell for which the interpolation will be calculated
			timeKya-> Time for which the climate will be calculated. Measured in kya. Must be any continuous value between 5000 and 0
		 If Time is a round number, than the number comes straight from the time series of PLASIM
		 If Time has a decimal value, than it is temporally interpolated between two milenia
		*/
		void getClimCell(int c, double timeKya, float SATMin, float SATMax, float PPTNMin, float PPTNMax, float NPP);
		void getClimCell(int c, int timeStep,  float SATMin, float SATMax, float PPTNMin, float PPTNMax, float NPP);			
																				
		// Interpolate the entire time series, for all cells of the model grid
		void getClimAtTime(double timeKya, TSngVector SATMin, TSngVector SATMax, TSngVector PPTNMin, TSngVector PPTNMax, TSngVector NPP);
		void getClimAtTime(int timeStep, TSngVector SATMin, TSngVector SATMax, TSngVector PPTNMin, TSngVector PPTNMax, TSngVector NPP);

		void getClimGrid(int timeStep, PSngVector envVec, PSngVector NPPVec);
		// Interpolate the entire time series, for all cells of the model grid
		void getClimTimeSeries(double startTime, double endTime, double timeResolution, 
							TSngMatrix SATMin, TSngMatrix SATMax, TSngMatrix PPTNMin, TSngMatrix PPTNMax, TSngMatrix NPP);


	};

}