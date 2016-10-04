#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <dirent.h> 
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <gps.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define Pi M_PI
float R = ((360 * 60) / (2 * Pi));
float nmi2ft = 6076.115485564300f;
float lat1 = 32.89876355058L; 
float lng1 = -106.15108144756f;
float bm1 = 5076.583f;
float lat3 = 33.02400244972f;
float lng3 = -106.16152893738f;
float bm3 = 50766.583f;

typedef struct ByRhA {
	float By;
	float Rh;
	float Angle;
} ByRhA_t;

float radtodeg(float rad);
ByRhA_t SphCoordDist(float lat1, float lng1, float lat2, float lng2);


int main() {

	printf("Earth Radius: %.6f\n",R);
	printf("Benchmark: %9.4f, Lat: %15.12f, Lng:%16.12f \n",bm1,lat1,lng1);
	printf("Benchmark: %9.4f, Lat: %15.12f, Lng:%16.12f \n",bm3,lat3,lng3);

	// By Rh and Angle from Coordinate set 1 to coordinate set 3
	ByRhA_t ByRhA = SphCoordDist(lat1, lng1, lat3, lng3);
	printf("By: %15.12f, Rh: %15.12f, angle: %15.12f \n",ByRhA.By,ByRhA.Rh,ByRhA.Angle);

	int rc;
	struct timeval tv;

	struct gps_data_t gps_data;
	if ((rc = gps_open("localhost", "2947", &gps_data)) == -1) {
		printf("code: %d, reason: %s\n", rc, gps_errstr(rc));
		return EXIT_FAILURE;
	}

	gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

	while (1) {
    		/* wait for 2 seconds to receive data */
    		if (gps_waiting (&gps_data, 2000000)) {
			/* read data */
			if ((rc = gps_read(&gps_data)) == -1) {
				printf("error occured reading gps data. code: %d, reason: %s\n", rc, gps_errstr(rc));
			}
			else {
            			/* Display data from the GPS receiver. */
				if ((gps_data.status == STATUS_FIX) && 
				    (gps_data.fix.mode == MODE_2D || gps_data.fix.mode == MODE_3D) &&
				    !isnan(gps_data.fix.latitude) && 
				    !isnan(gps_data.fix.longitude)) {

					gettimeofday(&tv, NULL);
					printf("latitude: %f, longitude: %f, speed: %f, timestamp: %ld\n", gps_data.fix.latitude, gps_data.fix.longitude, gps_data.fix.speed, tv.tv_sec);

				}
				else {
					printf("no GPS data available\n");
				}
			}
		}

		sleep(3);
	}

/* When you are done... */
gps_stream(&gps_data, WATCH_DISABLE, NULL);
gps_close (&gps_data);

return EXIT_SUCCESS;
}




float degtorad(float deg) {
	return (float) deg * Pi / 180;
}

float radtodeg(float rad) {
	return (float) rad / ( Pi / 180);
}

ByRhA_t  SphCoordDist(float lat1, float lng1, float lat2, float lng2) {

	float dlat = lat2 - lat1;

	float lat1r = degtorad(lat1);
	float lat2r = degtorad(lat2);
	float lng1r = degtorad(lng1);
	float lng2r = degtorad(lng2);

	float x1 = R * cos(lat1r) * cos(lng1r);
	float y1 = R * cos(lat1r) * sin(lng1r);
	float z1 = R * sin(lat1r);

	float x2 = R * cos(lat2r) * cos(lng2r);
	float y2 = R * cos(lat2r) * sin(lng2r);
	float z2 = R * sin(lat2r);


	float c = sqrt( pow((x2 - x1),2) + pow((y2 - y1),2) + pow((z2 - z1),2) );
	float C = radtodeg( acos( (pow(R,2) - 0.5 * pow(c,2)) / (pow(R,2)) ));

	float angle = C;
	float Rh = 60 * C;

	float Dlat = dlat * 60;

	float By;
	if (lng1 == lng2) {
		By = 0;
	}
	else {
		By = radtodeg( acos( dlat / Rh ) );
	}


	By = 90 - radtodeg( atan2( cos(lat1r) * sin(lat2r) - sin(lat1r) * cos(lat2r) * cos( lng2r-lng1r ),
	    sin( lng2r - lng1r ) * cos( lat2r ) ) );

	By = fmodf((By + 360), 360);


	ByRhA_t ByRhA;
	ByRhA.By = By;
	ByRhA.Rh = Rh;
	ByRhA.Angle = angle;

	return ByRhA;
}
