#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct rmc_sentence {
	time_t time_of_fix;
	char   receiver_status;
	double lat;
	double lon;
};

int twoCharsToNum(char *chars, int base, bool *ok) {
	
	char number[3] = {};
	number[0] = chars[0];
	number[1] = chars[1];
	
	char *endp;
	int value = strtol(number, &endp, base);
	if (ok != NULL) {
		if (endp == &number[2]) {
			*ok = true;
		} else {
			*ok = false;
		}
	}
	return value;
}

bool isNmeaLine(char *line, ssize_t linelen) {
	if (line[0] != '$')
		return false;
	if (line[linelen-5] != '*')
		return false;
	
	bool ok;
	uint8_t their_sum = twoCharsToNum(&line[linelen-4], 16, &ok);
	if (ok == false)
		return false;
	
	uint8_t our_sum = 0;
	int i;
	for(i = 1; line[i] != '*'; i++) {
		our_sum ^= line[i];
	}
		
	return our_sum == their_sum;
}

/* $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A */

bool decodeRmc(char *line, struct rmc_sentence *rmc) {
	struct tm t;
	
	int field_num = 0;
	for(char *token = strtok(line, ","); token; token = strtok(NULL, ","), field_num++) {
		switch(field_num) {
			case 0: /* Message ID $GPRMC */
			break;			
			case 1:	/* UTC of position fix */
			t.tm_hour = twoCharsToNum(token, 10, NULL);
			t.tm_min = twoCharsToNum(token+2, 10, NULL);
			t.tm_sec = twoCharsToNum(token+4, 10, NULL);
			break;
			case 2:	/* Status A=active or V=void */
			rmc->receiver_status = token[0];
			break;
			case 3:	/* Latitude */
			{
				int degree = twoCharsToNum(token, 10, NULL);
				double minutes = 0.0;
				double power = 0.1;
				int i;
				for(i = 2; token[i] != 0; i++) {
					if(token[i] == '.')
						continue;
					minutes += (token[i] - '0')/power;
					power *= 10;
				}
				rmc->lat = degree + minutes/60.0;
				
				token = strtok(NULL, ",");
				if(token[0] == 'S') {
					rmc->lat *= -1.0;
				}
			}
			break;
			case 4:	/* Longitude */
			{
				char degrees[4] = {};
				memcpy(degrees, token, 3);
				int degree = strtol(degrees, NULL, 10);
				
				double minutes = 0.0;
				double power = 0.1;
				int i;
				for(i = 3; token[i] != 0; i++) {
					if(token[i] == '.')
						continue;
					minutes += (token[i] - '0')/power;
					power *= 10;
				}
				rmc->lon = degree + minutes/60.0;
				
				token = strtok(NULL, ",");
				if(token[0] == 'E') {
					rmc->lat *= -1.0;
				}
			}
			break;
			case 5:	/* Speed over the ground in knots */
			/* Don't care */
			break;
			case 6:	/* Track angle in degrees (True) */
			/* Don't care */
			break;
			case 7:	/* Date */
			t.tm_mon = twoCharsToNum(token, 10, NULL);
			t.tm_mday = twoCharsToNum(token+2, 10, NULL);
			t.tm_year = twoCharsToNum(token+4, 10, NULL) + 100;
			rmc->time_of_fix = timegm(&t);
			break;
			case 8:	/* Magnetic variation in degrees */
			/* Don't care */
			break;
			case 9:	/* The checksum data, always begins with * */
			/* Don't care */
			break;
		}
			
	}
	
	return true;
}


int main(int argc, char **argv) {
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
	
    while ((linelen = getline(&line, &linecap, stdin)) > 0) {
		if (!isNmeaLine(line, linelen))
			continue;
		line[linelen-5] = 0;
		
		if (strncmp(line, "$GPRMC", 6) == 0) {
			struct rmc_sentence rmc;
			decodeRmc(line, &rmc);
			
			if(rmc.receiver_status == 'A') {
				struct tm *t = gmtime(&rmc.time_of_fix);
			
				printf("%02d:%02d:%02d, %f, %f\n", t->tm_hour, t->tm_min, t->tm_sec, 
											 rmc.lat, rmc.lon);
			}
		}
	}
	return 0;
	
}