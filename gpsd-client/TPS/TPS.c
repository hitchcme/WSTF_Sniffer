
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

#include <wiringPi.h>
#include <lcd.h>

#define APPEND 0
#define OVERWRITE 1

#define Pi 3.1415926535897932384626433832795029L
//double R = (float) ((360 * 60) / (2 * Pi));

//sealevel radius at equator
double r1 = 3443.9184665227L;
//sealevel radius at poles
double r2 = 3432.37149028L;

// Unit conversions
double nmi2ft = 6076.115485564300L;
double m2ft = 3.2808398950131L;

//struct gps_data_t gps_data;

typedef struct ByRhA {
	double By;
	double Rh;
	double Angle;
} ByRhA_t;

typedef struct Vels {
	double N;
	double E;
} Vels_t;

typedef struct Errors {
	double NS;
	double EW;
} Errors_t;

typedef struct TTPos {
	double TS;
	double Offset;
	Vels_t vel;
	Errors_t errors;
} TTPos_t;

typedef struct DMS {
	int deg;
	int min;
	double sec;
} DMS_t;

typedef struct BMC {
	double lat1;
	DMS_t lat1_dms;
	double lng1;
	DMS_t lng1_dms;
	double bm1;
	double alt1;

	double lat2;
	DMS_t lat2_dms;
	double lng2;
	DMS_t lng2_dms;
	double bm2;
	double alt2;

	ByRhA_t ByRhA;
	double DistGain;
	double avg_Radius;

} BMC_t;

typedef struct CurrGPSdata {
	double lat;
	DMS_t lat_dms;
	double lng;
	DMS_t lng_dms;
	double alt;
	double course;
	double speed;
	Vels_t vel;
	Errors_t errors;

} CurrGPSdata_t;

typedef struct CurrTPSdata {
	double TS;
	double Offset;
	Vels_t vel;
	Errors_t errors;
} CurrTPSdata_t;


double lat_arr[100];
double lng_arr[100];
double alt_arr[100];
double Vel_N_arr[100];
double Vel_E_arr[100];
double Err_NS_arr[100];
double Err_EW_arr[100];
double TS_arr[100];
double Offset_arr[100];
double RelVel_N_arr[100];
double RelVel_E_arr[100];
double RelErr_NS_arr[100];
double RelErr_EW_arr[100];


// functions
double rad2deg(double rad);
double deg2rad(double deg);
double Earth_radius_at_sealevel( double lat );
ByRhA_t SphCoordDist( double R, double lat1, double lng1, double lat2, double lng2);
Vels_t Vectorize_Course_Speed( double Course, double Speed);
Errors_t Rotate_Errors(double BaseAngle, double LatErr, double LngErr);
BMC_t GetBencMarkCoords();
CurrGPSdata_t keep_CurrentGPSDat(double lat, double lng, double alt, double course, double speed, double laterr, double lngerr);
DMS_t D2DMS(double DEG);
TTPos_t Calculate_Track_Position(CurrGPSdata_t DATA, BMC_t BMC);
void Write_to_webKML(char *PATH, int append, CurrGPSdata_t DATA, BMC_t BMC, int iteration);
void WriteTPSArray( CurrGPSdata_t DATA, TTPos_t DATA1 );
//double R;

int main() {

	FILE *FID;
	FID = fopen("/dev/ttyAMA0", "w");
	//fprintf(FID, "$PMTK301,2*2E\r\n");
	//sleep(0.1);
	fprintf(FID, "$PMTK313,1*2E\r\n");
	sleep(0.1);
	fprintf(FID, "$PMTK301,2*2E\r\n");
	sleep(0.1);
	fprintf(FID, "$PMTK220,200*2C\r\n");
	sleep(0.1);
	fclose(FID);

	int iteration = 0;

	int rc;
	struct timeval tv;

	struct gps_data_t gps_data;
	if ((rc = gps_open("localhost", "2947", &gps_data)) == -1) {
		printf("code: %d, reason: %s\n", rc, gps_errstr(rc));
		return EXIT_FAILURE;
	}

	gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);
	int ROWS = 2; int COLS = 16; int BITS = 4; int RS = 15; int EN = 13;
	int D4 = 12; int D5 = 11; int D6 = 10; int D7 = 9;
	int fd2 = lcdInit(ROWS, COLS, BITS, RS, EN, D4, D5, D6, D7, 0,0,0,0);

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

					if (iteration < 100) {
						iteration++;
					}
					gettimeofday(&tv, NULL);
					char datetime_str[100];
					time_t now = time(0);
					strftime(datetime_str, 100, "%H:%M:%S", localtime(&now));
					sprintf(datetime_str,"%s.%06i",datetime_str,tv.tv_usec);
					printf("%s\n", datetime_str);

					BMC_t BMC = GetBencMarkCoords();
					printf("Bench Mark Locations:\n");
					printf("	Latitude: %16.11lf, Longitude: %16.11lf, Altitude: %.1f, BenchMark: %9.3f\n", 
						BMC.lat2, BMC.lng2, BMC.alt2, BMC.bm2);
                                        printf("	Latitude: %16.11lf, Longitude: %16.11lf, Altitude: %.1f, BenchMark: %9.3f\n", 
						BMC.lat1, BMC.lng1, BMC.alt1, BMC.bm1);
					printf("	DistGain: %16.11lf \n",BMC.DistGain);
					printf("	avgRadius: %16.11lf \n",BMC.avg_Radius);
					printf("	Calculated Distance: %16.11lf \n",BMC.ByRhA.Rh*nmi2ft);
					printf("	deltaBM:	%9.4lf \n",BMC.bm2-BMC.bm1);

					double latx = gps_data.fix.latitude;
					double lngx = gps_data.fix.longitude;
					double altx = gps_data.fix.altitude*m2ft;

					CurrGPSdata_t DATA = keep_CurrentGPSDat(	gps_data.fix.latitude,// + 0.62777200015L,
											gps_data.fix.longitude,// + 0.63365480753L,
											gps_data.fix.altitude * m2ft,
											gps_data.fix.track,
											gps_data.fix.speed * m2ft,
											gps_data.fix.epx * m2ft,
											gps_data.fix.epy * m2ft );
					printf("Current GPS Location:\n");
					printf("	Latitude:		  Longitude:\n");
					printf("	%15.11lf,	   %16.11lf \n",DATA.lat,DATA.lng);
					printf("	%3d\u00B0 %2d' %14.12lf\"   %3d\u00B0 %2d' %14.12lf\" \n",DATA.lat_dms.deg,DATA.lat_dms.min,DATA.lat_dms.sec,DATA.lng_dms.deg,DATA.lng_dms.min,DATA.lng_dms.sec);
					printf("	Course: %6.2lf\u00B0, Speed: %10.3lf ft/s \n", DATA.course, DATA.speed);
					printf("	Velocities: N,E \n");
					printf("		%16.11lf ft/s, %16.11lf ft/s \n",DATA.vel.N,DATA.vel.E);
					printf("	Error: NS, EW\n");
					printf("		\u00B1%6.3lf ft, \u00B1%6.3lf ft \n",DATA.errors.NS,DATA.errors.EW);

					TTPos_t TTPos = Calculate_Track_Position(DATA, BMC);

					WriteTPSArray( DATA, TTPos);

					Write_to_webKML("/var/www/html/TPS.kml",APPEND,DATA, BMC, iteration);

					int lcdTS = floor((double) TTPos.TS + 0.50000000000000f);
					int lcdOffset = floor((double) TTPos.Offset + 0.500000000000f);

					char lcdOffset_str[30];
					if ( lcdOffset < 0.00000000000L ) {
						sprintf(lcdOffset_str, "W %i ft              ", abs(lcdOffset));
					}
					else {
						sprintf(lcdOffset_str, "E %i ft              ", abs(lcdOffset));
					}

					char lcdTS_str[30];
					if (lcdTS >= 0.000000000000000L) {
						sprintf(lcdTS_str,"TS: %i ft             ",lcdTS);
					}
					else {
						sprintf(lcdTS_str,"TS:  %i ft            ",lcdTS);
					}

					printf("    TS: %lf	=>	%i \n", TTPos.TS, lcdTS );
					printf("Offset: %lf 	=>	%i \n", TTPos.Offset, lcdOffset );
					printf("%s \n",lcdTS_str);
					printf("    %s \n",lcdOffset_str);

					lcdClear(fd2);
					lcdPosition(fd2,0,0);
					lcdPuts(fd2,lcdTS_str);
					lcdPuts(fd2,"          ");
					lcdPosition(fd2,0,1);
					lcdPuts(fd2,"  ");
					lcdPuts(fd2,lcdOffset_str);
					lcdPuts(fd2,"          ");
					printf("------------------------------------------------------------------------------------------------------------\n");

				}
				else {
					printf("no GPS data available\n");
				}
			}
		}

		//sleep(0.1);
	}

/* When you are done... */
gps_stream(&gps_data, WATCH_DISABLE, NULL);
gps_close (&gps_data);

return EXIT_SUCCESS;
}




double deg2rad( double deg ) {
	return (double) deg * Pi / 180;
}

double rad2deg( double rad ) {
	return (double) rad / ( Pi / 180);
}

ByRhA_t  SphCoordDist( double R, double lat1, double lng1, double lat2, double lng2) {

	double dlat = lat2 - lat1;

	double lat1r = deg2rad(lat1);
	double lat2r = deg2rad(lat2);
	double lng1r = deg2rad(lng1);
	double lng2r = deg2rad(lng2);

	double x1 = R * cos(lat1r) * cos(lng1r);
	double y1 = R * cos(lat1r) * sin(lng1r);
	double z1 = R * sin(lat1r);

	double x2 = R * cos(lat2r) * cos(lng2r);
	double y2 = R * cos(lat2r) * sin(lng2r);
	double z2 = R * sin(lat2r);


	double c = sqrt( pow((x2 - x1),2) + pow((y2 - y1),2) + pow((z2 - z1),2) );
	double C = rad2deg( acos( (pow(R,2) - 0.5 * pow(c,2)) / (pow(R,2)) ));

	double angle = C;
	double Rh = 60 * C;

	double Dlat = dlat * 60;

	double By;
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

double Earth_radius_at_sealevel( double lat ) {
	// https://rechneronline.de/earth-radius/
	double lrad = deg2rad(lat);

	double A1 = ( r1 * r1 * cos(lrad) ) * ( r1 * r1 * cos(lrad) );
	double A2 = ( r2 * r2 * sin(lrad) ) * ( r2 * r2 * sin(lrad) );
	double B1 = ( r1 * cos(lrad) ) * ( r1 * cos(lrad) );
	double B2 = ( r2 * sin(lrad) ) * ( r2 * sin(lrad) );

	return (double) sqrt( ( A1 + A2 ) / ( B1 + B2 ) );
}

Vels_t Vectorize_Course_Speed( double Course, double Speed) {

	Vels_t VectVels;
	VectVels.N = Speed * cos( deg2rad( Course ) );
	VectVels.E = Speed * sin( deg2rad( Course ) );
	return VectVels;

}

Errors_t Rotate_Errors(double BaseAngle, double LatErr, double LngErr) {

	double angle = rad2deg( atan2( LngErr, LatErr ) );
	double length = sqrt( powf(LatErr,2) + powf(LngErr,2) );
	double angle_corr = angle - 360 - BaseAngle;

	Errors_t Errors;
	Errors.NS = length * cos( deg2rad( angle_corr ) );
	Errors.EW = length * sin( deg2rad( angle_corr ) );
	//Errors.EW = sqrt( powf(length,2) - powf(Errors.NS,2) );
//	Errors.NS = Errors.NS * m2ft;
//	Errors.EW = Errors.EW * m2ft;
	return Errors;
}

BMC_t GetBencMarkCoords() {

	FILE *FID = fopen("/etc/TPS/Settings","r");

	char lat1_str[100] = {0}; char lat2_str[100] = {0}; char lng1_str[100] = {0}; char lng2_str[100] = {0};
	char alt1_str[100] = {0}; char alt2_str[100] = {0}; char bm1_str[100] = {0}; char bm2_str[100] = {0};

	fscanf(FID, "%[^, ], %[^, ], %[^, ], %[^, ], %[^, ], %[^, ], %[^, ], %[^\n]", lat1_str, lng1_str, alt1_str, bm1_str, lat2_str, lng2_str, alt2_str, bm2_str);

	fclose(FID);

	BMC_t BMC;
	BMC.lat1 = atof(lat1_str);
	BMC.lat1_dms = D2DMS(BMC.lat1);
	BMC.lat2 = atof(lat2_str);
	BMC.lat2_dms = D2DMS(BMC.lat2);
	BMC.lng1 = atof(lng1_str);
	BMC.lng1_dms = D2DMS(BMC.lng1);
	BMC.lng2 = atof(lng2_str);
	BMC.lng2_dms = D2DMS(BMC.lng2);
	BMC.alt1 = atof(alt1_str);
	BMC.alt2 = atof(alt2_str);
	BMC.bm1 = atof(bm1_str);
	BMC.bm2 = atof(bm2_str);

	BMC.avg_Radius = Earth_radius_at_sealevel((BMC.lat1 + BMC.lat2)/2);
	BMC.ByRhA = SphCoordDist(BMC.avg_Radius, BMC.lat1, BMC.lng1, BMC.lat2, BMC.lng2);
        BMC.DistGain = (BMC.bm2 - BMC.bm1) / (BMC.ByRhA.Rh * nmi2ft);

	return BMC;
}

DMS_t D2DMS(double DEG) {

	double absDEG = sqrt(DEG * DEG);
	int deg = (int) floor(absDEG);
	int min = (int) floor((absDEG - deg)*60);
	double sec = (double) (((absDEG - deg)*60.000000000000L)-min)*60.000000000000L;

	DMS_t DMS;
	DMS.deg = (int) deg * ( (double) DEG / absDEG );
	DMS.min = min;
	DMS.sec = sec;
	return DMS;
}

CurrGPSdata_t keep_CurrentGPSDat(double lat, double lng, double alt, double course, double speed, double laterr, double lngerr) {

	CurrGPSdata_t Curr_GPS_Data;

	Curr_GPS_Data.lat = lat;
	Curr_GPS_Data.lat_dms = D2DMS(lat);

	Curr_GPS_Data.lng = lng;
	Curr_GPS_Data.lng_dms = D2DMS(lng);

	Curr_GPS_Data.alt = alt;
	Vels_t vel = Vectorize_Course_Speed( course, speed );
	Curr_GPS_Data.vel.N = vel.N;
	Curr_GPS_Data.vel.E = vel.E;

	Curr_GPS_Data.errors.NS = laterr;
	Curr_GPS_Data.errors.EW = lngerr;

	Curr_GPS_Data.course = course;
	Curr_GPS_Data.speed = speed;

	return Curr_GPS_Data;

}

TTPos_t Calculate_Track_Position(CurrGPSdata_t DATA, BMC_t BMC) {

	struct timeval tv;

	double R = BMC.avg_Radius;
	double theta12 = deg2rad(BMC.ByRhA.By);
	double d12 = deg2rad(BMC.ByRhA.Angle);
	double Dist12 = BMC.ByRhA.Rh;

	int STICKYcalc = 0;
	ByRhA_t ByRhA1;
	if ( DATA.lat > BMC.lat1 + (BMC.lat2 - BMC.lat1) / 2 ) {
		ByRhA1 = SphCoordDist(BMC.avg_Radius, BMC.lat1, BMC.lng1, DATA.lat, DATA.lng);
		STICKYcalc = 0;
	}
	else {
		ByRhA1 = SphCoordDist(BMC.avg_Radius, BMC.lat2, BMC.lng2, DATA.lat, DATA.lng);
		STICKYcalc = 1;
	}

	double d13 = deg2rad(ByRhA1.Angle);
	double theta13 = deg2rad(ByRhA1.By);
	double Dist13 = ByRhA1.Rh;

	double dxt = asin( ( sin( d13 / R ) ) * ( sin( theta13 - theta12)) ) * R;
	double dat = acos( ( cos( d13 / R ) ) / ( cos( dxt / R ) )  ) * R;

	double offset = rad2deg( dxt / R) * R * 60 * nmi2ft * BMC.DistGain - 1.0000000000000L;
	double TS = rad2deg( dat / R) * R * 60 * nmi2ft * BMC.DistGain;

	if (STICKYcalc == 1) {
		TS = BMC.bm2 - TS;
	}
	else {
		TS = BMC.bm1 + TS;
	}

	// We've got position at some time, now we can predict the next position
	double relCourse = BMC.ByRhA.By - 360 + DATA.course;
	Vels_t relVels = Vectorize_Course_Speed( relCourse, DATA.speed );
	Errors_t relErrs = Rotate_Errors( BMC.ByRhA.By, DATA.errors.NS, DATA.errors.EW );

	gettimeofday(&tv, NULL);
	time_t now = time(0);
	//sprintf(datetime_str,"%s.%06i",datetime_str,tv.tv_usec);
	//float usec = (float) tv.tv_usec / 1000000.0000L);


	TTPos_t TTPos;
	TTPos.TS = TS + (relVels.N * tv.tv_usec / 1000000.0000L);
	TTPos.vel = relVels;
	TTPos.errors = relErrs;
	//TTPos.TS = TS;
	TTPos.Offset = offset + (relVels.E * tv.tv_usec / 1000000.0000L);
	//TTPos.Offset = offset;

	printf("TS (before vel): 	%16.11lf \n",TS);
	printf("Offset (before vel):	%16.11lf \n",offset);

	return TTPos;

}


void Write_to_webKML(char *PATH, int append, CurrGPSdata_t DATA, BMC_t BMC, int iteration) {

	FILE *KMLFID;
	//if (append = APPEND) {
	//	KMLFID = fopen(PATH, "a");
	//}
	//else {
		KMLFID = fopen(PATH, "w");
		fprintf(KMLFID,"%s\n","<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		fprintf(KMLFID,"%s\n","<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">");
		fprintf(KMLFID,"%s\n","<Document>");
		fprintf(KMLFID,"%s\n","	<name>TPS.kml</name>");
		fprintf(KMLFID,"%s\n","	<StyleMap id=\"m_ylw-pushpin\">\n");
		fprintf(KMLFID,"%s\n","		<Pair>");
		fprintf(KMLFID,"%s\n","			<key>normal</key>");
		fprintf(KMLFID,"%s\n","			<styleUrl>#s_ylw-pushpin</styleUrl>");
		fprintf(KMLFID,"%s\n","		</Pair>");
		fprintf(KMLFID,"%s\n","		<Pair>");
		fprintf(KMLFID,"%s\n","			<key>highlight</key>");
		fprintf(KMLFID,"%s\n","			<styleUrl>#s_ylw-pushpin_hl</styleUrl>");
		fprintf(KMLFID,"%s\n","		</Pair>");
		fprintf(KMLFID,"%s\n","	</StyleMap>");
		fprintf(KMLFID,"%s\n","	<Style id=\"s_ylw-pushpin\">");
		fprintf(KMLFID,"%s\n","		<IconStyle>");
		fprintf(KMLFID,"%s\n","			<scale>1.1</scale>");
		fprintf(KMLFID,"%s\n","			<Icon>");
		fprintf(KMLFID,"%s\n","				<href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href>");
		fprintf(KMLFID,"%s\n","			</Icon>");
		fprintf(KMLFID,"%s\n","			<hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" yunits=\"pixels\"/>");
		fprintf(KMLFID,"%s\n","		</IconStyle>");
		fprintf(KMLFID,"%s\n","	</Style>");
		fprintf(KMLFID,"%s\n","	<Style id=\"s_ylw-pushpin_hl\">");
		fprintf(KMLFID,"%s\n","		<IconStyle>");
		fprintf(KMLFID,"%s\n","			<scale>1.3</scale>");
		fprintf(KMLFID,"%s\n","			<Icon>");
		fprintf(KMLFID,"%s\n","				<href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href>");
		fprintf(KMLFID,"%s\n","			</Icon>");
		fprintf(KMLFID,"%s\n","			<hotSpot x=\"20\" y=\"2\" xunits=\"pixels\" yunits=\"pixels\"/>");
		fprintf(KMLFID,"%s\n","		</IconStyle>");
		fprintf(KMLFID,"%s\n","	</Style>");
		fprintf(KMLFID,"%s\n","	<Placemark>");
		fprintf(KMLFID,"%s\n","		<name>Test Track</name>");
		fprintf(KMLFID,"%s\n","		<styleUrl>#m_ylw-pushpin</styleUrl>");
		fprintf(KMLFID,"%s\n","		<LineString>");
		fprintf(KMLFID,"%s\n","			<tessellate>1</tessellate>");
		fprintf(KMLFID,"%s\n","			<coordinates>");
		fprintf(KMLFID,"				%16.11lf,%14.11lf,%.1lf\n",BMC.lng1,BMC.lat1,BMC.alt1);
		fprintf(KMLFID,"				%16.11lf,%14.11lf,%.1lf\n",BMC.lng2,BMC.lat2,BMC.alt2);
		fprintf(KMLFID,"%s\n","			</coordinates>");
		fprintf(KMLFID,"%s\n","		</LineString>");
		fprintf(KMLFID,"%s\n","	</Placemark>");
		fprintf(KMLFID,"\n");
		fprintf(KMLFID,"%s\n","	<Placemark>");
		fprintf(KMLFID,"%s\n","		<name>Position</name>");
		fprintf(KMLFID,"%s\n","		<styleUrl>#m_ylw-pushpin</styleUrl>");
		fprintf(KMLFID,"%s\n","		<LineString>");
		fprintf(KMLFID,"%s\n","			<tessellate>1</tessellate>");
		fprintf(KMLFID,"%s\n","			<coordinates>");
		for (int i = 1; i <= iteration - 1; i++) {
			fprintf(KMLFID,"				%16.11lf,%14.11lf,%.1lf\n",lng_arr[i],lat_arr[i],alt_arr[i]);
		}
		fprintf(KMLFID,"%s\n","			</coordinates>");
		fprintf(KMLFID,"%s\n","		</LineString>");
		fprintf(KMLFID,"%s\n","	</Placemark>");
	//}



	fprintf(KMLFID,"</Document>\n");
	fprintf(KMLFID,"</kml>\n");
	fclose(KMLFID);

}

void WriteTPSArray( CurrGPSdata_t DATA, TTPos_t DATA1 ) {
	for (int i = 99; i >= 0; i--) {
		lat_arr[i]	= lat_arr[i-1];
		lng_arr[i]	= lng_arr[i-1];
		alt_arr[i]	= alt_arr[i-1];
		Vel_N_arr[i]	= Vel_N_arr[i-1];
		Vel_E_arr[i]	= Vel_E_arr[i-1];
		Err_NS_arr[i]	= Err_NS_arr[i-1];
		Err_EW_arr[i]	= Err_EW_arr[i-1];
		TS_arr[i]	= TS_arr[i-1];
		Offset_arr[i]	= Offset_arr[i-1];
		RelVel_N_arr[i]	= RelVel_N_arr[i-1];
		RelVel_E_arr[i]	= RelVel_E_arr[i-1];
		RelErr_NS_arr[i]= RelErr_NS_arr[i-1];
		RelErr_EW_arr[i]= RelErr_EW_arr[i-1];
	}

	lat_arr[0]	= DATA.lat;
	lng_arr[0]	= DATA.lng;
	alt_arr[0]	= DATA.alt;
	Vel_N_arr[0]	= DATA.vel.N;
	Vel_E_arr[0]	= DATA.vel.E;
	Err_NS_arr[0]	= DATA.errors.NS;
	Err_EW_arr[0]	= DATA.errors.EW;

	TS_arr[0]	= DATA1.TS;
	Offset_arr[0]	= DATA1.Offset;
	RelVel_N_arr[0]	= DATA1.vel.N;
	RelVel_E_arr[0]	= DATA1.vel.E;
	RelErr_NS_arr[0]= DATA1.errors.NS;
	RelErr_EW_arr[0]= DATA1.errors.EW;

//	for (int i = 0; i <= 99; i++) {
//		printf("Meas: %3d, lat: %14.11lf, lng: %16.11lf, alt: %0.6lf, Vels(N,E): (%0.6lf, %0.6lf), Errs(NS,EW): (%0.6lf, %0.6lf) \n", i, lat_arr[i], lng_arr[i], alt_arr[i], Vel_N_arr[i], Vel_E_arr[i], Err_NS_arr[i], Err_EW_arr[i] );
//	}
}
