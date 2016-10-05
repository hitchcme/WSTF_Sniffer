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

#define Pi 3.1415926535897932384626433832795029L
//float R = (float) ((360 * 60) / (2 * Pi));

//sealevel radius at equator
float r1 = 3443.9184665227f;
//sealevel radius at poles
float r2 = 3432.37149028f;

float nmi2ft = 6076.115485564300f;
float lat1 = 32.89876355058f; 
float lng1 = -106.15108144756f;
float bm1 = 5076.583f;
float lat3 = 33.02400244972f;
float lng3 = -106.16152893738f;
float bm3 = 50766.583f;
float avg_altitude = 4080;


typedef struct ByRhA {
	float By;
	float Rh;
	float Angle;
} ByRhA_t;


float rad2deg(float rad);
float deg2rad(float deg);
float Earth_radius_at_sealevel( float lat );
ByRhA_t SphCoordDist( float R, float lat1, float lng1, float lat2, float lng2);

float R;

int main() {

	float avg_lat = (float) (lat1 + lat3) / 2;
	float R = Earth_radius_at_sealevel( avg_lat ) + (avg_altitude / nmi2ft);

	printf("Pi: %f\n",Pi);
	printf("Earth Radius: %15.6f\n",R);
	printf("r1: %15.6f,	r2: %15.6f \n",r1,r2);
	printf("Benchmark: %9.4f, Lat: %15.12f, Lng:%16.12f \n",bm1,lat1,lng1);
	printf("Benchmark: %9.4f, Lat: %15.12f, Lng:%16.12f \n",bm3,lat3,lng3);

	// By Rh and Angle from Coordinate set 1 to coordinate set 3
	ByRhA_t ByRhA = SphCoordDist(R, lat1, lng1, lat3, lng3);
	printf("By: %15.12f, Rh: %15.12f, angle: %15.12f \n",ByRhA.By,ByRhA.Rh,ByRhA.Angle);
	float theta12 = deg2rad(ByRhA.By);
	float d12 = ByRhA.Angle;
	float Dist12 = ByRhA.Rh;

	float deltaBM = bm3 - bm1;
        float DistGain = deltaBM / (Dist12 * nmi2ft);

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

					// Calculate relative position
					float latx = gps_data.fix.latitude;
					float lngx = gps_data.fix.longitude;

					int STICKYcalc = 0;
					ByRhA_t ByRhA1;
					if ( latx > lat1 + (lat3 - lat1) / 2 ) {
						ByRhA1 = SphCoordDist(R, lat1, lng1, latx, lngx);
						STICKYcalc = 0;
					}
					else {
						ByRhA1 = SphCoordDist(R, lat3, lng3, latx, lngx);
						STICKYcalc = 1;
					}
					float d13 = ByRhA1.Angle;
					float theta13 = ByRhA1.By;
					float Dist13 = ByRhA1.Rh;

					d13 = deg2rad(d13);
					theta13 = deg2rad(theta13);

					float dxt = asin( ( sin( d13 / R ) ) * ( sin( theta13 - theta12)) ) * R;
					float dat = acos( ( cos( d13 / R ) ) / ( cos( dxt / R ) )  ) * R;

					float offset = rad2deg( dxt / R) * R * 60 * nmi2ft * DistGain;
					float TS = rad2deg( dat / R) * R * 60 * nmi2ft * DistGain;

					if (STICKYcalc == 1) {
						TS = bm3 - TS;
					}
					else {
						TS = bm1 + TS;
					}

					printf("    TS: %f\n",TS);
					printf("Offset: %f\n",offset);

				}
				else {
					printf("no GPS data available\n");
				}
			}
		}

		sleep(0.1);
	}

/* When you are done... */
gps_stream(&gps_data, WATCH_DISABLE, NULL);
gps_close (&gps_data);

return EXIT_SUCCESS;
}




float deg2rad( float deg ) {
	return (float) deg * Pi / 180;
}

float rad2deg( float rad ) {
	return (float) rad / ( Pi / 180);
}

ByRhA_t  SphCoordDist( float R, float lat1, float lng1, float lat2, float lng2) {

	float dlat = lat2 - lat1;

	float lat1r = deg2rad(lat1);
	float lat2r = deg2rad(lat2);
	float lng1r = deg2rad(lng1);
	float lng2r = deg2rad(lng2);

	float x1 = R * cos(lat1r) * cos(lng1r);
	float y1 = R * cos(lat1r) * sin(lng1r);
	float z1 = R * sin(lat1r);

	float x2 = R * cos(lat2r) * cos(lng2r);
	float y2 = R * cos(lat2r) * sin(lng2r);
	float z2 = R * sin(lat2r);


	float c = sqrt( pow((x2 - x1),2) + pow((y2 - y1),2) + pow((z2 - z1),2) );
	float C = rad2deg( acos( (pow(R,2) - 0.5 * pow(c,2)) / (pow(R,2)) ));

	float angle = C;
	float Rh = 60 * C;

	float Dlat = dlat * 60;

	float By;
	if (lng1 == lng2) {
		By = 0;
	}
	else {
		By = rad2deg( acos( dlat / Rh ) );
	}


	By = 90 - rad2deg( atan2( cos(lat1r) * sin(lat2r) - sin(lat1r) * cos(lat2r) * cos( lng2r-lng1r ),
	    sin( lng2r - lng1r ) * cos( lat2r ) ) );

	By = fmodf((By + 360), 360);


	ByRhA_t ByRhA;
	ByRhA.By = By;
	ByRhA.Rh = Rh;
	ByRhA.Angle = angle;

	return ByRhA;
}

float Earth_radius_at_sealevel( float lat ) {
	// https://rechneronline.de/earth-radius/
	float lrad = deg2rad(lat);

	float A1 = ( r1 * r1 * cos(lrad) ) * ( r1 * r1 * cos(lrad) );
	float A2 = ( r2 * r2 * sin(lrad) ) * ( r2 * r2 * sin(lrad) );
	float B1 = ( r1 * cos(lrad) ) * ( r1 * cos(lrad) );
	float B2 = ( r2 * sin(lrad) ) * ( r2 * sin(lrad) );

	return (float) sqrt( ( A1 + A2 ) / ( B1 + B2 ) );
}
