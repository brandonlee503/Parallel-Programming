#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Globals
int	NowYear;		// 2017 - 2022
int	NowMonth;		// 0 - 11

float NowPrecip;	// inches of rain per month
float NowTemp;		// temperature this month
float NowHeight;	// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int Storm;

unsigned int seed = 0;  // a thread-private variable

const float GRAIN_GROWS_PER_MONTH   = 8.0;
const float ONE_DEER_EATS_PER_MONTH = 0.5;

const float AVG_PRECIP_PER_MONTH    = 6.0;	// average
const float AMP_PRECIP_PER_MONTH    = 6.0;	// plus or minus
const float RANDOM_PRECIP           = 2.0;	// plus or minus noise

const float AVG_TEMP                = 50.0;	// average
const float AMP_TEMP                = 20.0;	// plus or minus
const float RANDOM_TEMP             = 10.0;	// plus or minus noise

const float MIDTEMP                 = 40.0;
const float MIDPRECIP               = 10.0;

const int ENDYEAR                   = 2023;

// Function Prototypes
float SQR(float x);
float Ranf( unsigned int *seedp, float low, float high );
int Ranf( unsigned int *seedp, int ilow, int ihigh );
float getAng();
float getTemp();
float getPrecip();
void GrainDeer();
void Grain();
void TornadoStorm();
void Watcher();


int main( int argc, char *argv[] ) {
	omp_set_num_threads( 4 );

	// starting date and time:
	NowMonth =    0;
	NowYear  = 2017;

	// starting state (feel free to change this if you want):
	NowNumDeer = 1;
	NowHeight =  1.;

	#pragma omp parallel sections
    {
        #pragma omp section
        {
            GrainDeer();
        }

        #pragma omp section
        {
            Grain();
        }

        #pragma omp section
        {
            TornadoStorm();
        }

        #pragma omp section
        {
            Watcher();
        }
    }

    return 0;
}

float SQR( float x ) {
        return x*x;
}

float Ranf( unsigned int *seedp, float low, float high ) {
        float r = (float) rand_r( seedp ); // 0 - RAND_MAX

        return( low + r * ( high - low ) / (float)RAND_MAX );
}

int Ranf( unsigned int *seedp, int ilow, int ihigh ) {
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)( Ranf(seedp, low,high) );
}

float getAng() {
    return (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
}

float getTemp() {
    float temp = AVG_TEMP - AMP_TEMP * cos( getAng() );
    unsigned int seed = 0;
    return temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );
}

float getPrecip() {
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( getAng() );
    precip += Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if ( precip < 0. ) {
    	precip = 0.;
    }

    return precip;
}

void GrainDeer() {
    int spawnDeer, deadDeer;
    while (NowYear <= ENDYEAR) {
        spawnDeer = 0;
        deadDeer = 0;

        // Compute temp variable
        if (Storm) {
            deadDeer = Ranf(&seed, 0, NowNumDeer-1);
        }

        // Deer population dependent on grain supply
        if (NowNumDeer <= NowHeight) {
            deadDeer++;
        } else {
            spawnDeer++;
        }

        // DoneComputing
        #pragma omp barrier
        NowNumDeer += spawnDeer;
        NowNumDeer -= deadDeer;

        // Ensure population is non negative
        if (NowNumDeer <= 0) {
            NowNumDeer = 1;
        }

        // DoneAssigning
        #pragma omp barrier

        // DonePrinting
        #pragma omp barrier
    }
}

void Grain() {
	int deadGrain;
	while (NowYear <= ENDYEAR) {
		float tempFactor   = exp( -SQR( ( NowTemp - MIDTEMP ) / 10. ) );
		float precipFactor = exp( -SQR( ( NowPrecip - MIDPRECIP ) / 10. ) );

		// Compute temp variable
		if (Storm) {
			deadGrain = NowHeight - Ranf(&seed, 0., NowHeight-1);
		}

		// DoneComputing
		#pragma omp barrier

		NowHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
	    NowHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

		if (NowHeight < 0) {
			NowHeight = .5;
		} else if (NowHeight > 100) {
			NowHeight = 50;
		}

		// DoneAssigning
		#pragma omp barrier

		// DonePrinting
		#pragma omp barrier
	}
}

void TornadoStorm() {
	while (NowYear <= ENDYEAR) {
		int tempStorm;

		// Storm happens when conditions are met
		if (getPrecip() > AVG_PRECIP_PER_MONTH && getTemp() < AVG_TEMP) {
			tempStorm = 1;
		} else {
			tempStorm = 0;
		}

		// DoneComputing
		#pragma omp barrier

		Storm = tempStorm;
		// DoneAssigning
		#pragma omp barrier

		// DonePrinting
		#pragma omp barrier
	}
}

void Watcher() {
	printf("Date | Temp | Precip | Deer | Grain | Fire\n");
    int updateYear, updateMonth, updateTemp, updatePrecip;
    while (NowYear <= ENDYEAR) {
        updateMonth = NowMonth + 1;
		if (updateMonth > 11) {
			updateYear = NowYear + 1;
			updateMonth = 0;
		} else {
			updateYear = NowYear;
		}

		updateTemp = getTemp();
		updatePrecip = getPrecip();

		// DoneComputing
		#pragma omp barrier

		NowYear = updateYear;
		NowMonth = updateMonth;
		NowTemp = updateTemp;
		NowPrecip = updatePrecip;

		// DoneAssigning
		#pragma omp barrier
		printf("%d/%d | %f | %f | %d | %f | %d\n", NowMonth + 1, NowYear, NowTemp, NowPrecip, NowNumDeer, NowHeight, Storm);

		// DonePrinting
		#pragma omp barrier
    }
}
