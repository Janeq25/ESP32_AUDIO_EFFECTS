#include "effects.h"

#include "defines.h"
#include "globals.h"
#include "util.h"
#include "math.h"





int16_t delay(int16_t in, uint16_t length){
    int16_t out;
    set_buffer_delay(length, &buffer1);

    buffer_set(in, &buffer1);

    out = in + buffer_get(&buffer1);

    return out;
}

int16_t echo(int16_t in, uint16_t length, float sustain){
    int16_t out;
    set_buffer_delay(length, &buffer1);

    out = in + (int16_t)(buffer_get(&buffer1) * sustain);

    buffer_set(out, &buffer1);

    return out;
}



#define FIR_N 9

float fir_f_coefs[] = {-0.008619748131390038,
0.058451474662495818,
0.137277859715206785,
0.200538222317029891,
0.224704382873315123,
0.200538222317029891,
0.137277859715206785,
0.058451474662495818,
-0.008619748131390038};

float fir_buffer[FIR_N] = {0};
int fir_buffer_ptr = 0;

int16_t FIR_f(int16_t input){
	float accumulator = 0;
	int i;
	fir_buffer[fir_buffer_ptr] = (float)input;

	
	for (i = 0; i < FIR_N; i++){
		accumulator += fir_buffer[(fir_buffer_ptr+i+1)%FIR_N] * fir_f_coefs[i];
	}
	
	fir_buffer_ptr++;
	if (fir_buffer_ptr >= FIR_N) fir_buffer_ptr = 0;
	
	return accumulator;
}

int16_t echo_fir(int16_t in, uint16_t length, float sustain){
    int16_t out;
    set_buffer_delay(length, &buffer1);

    out = in + FIR_f((int16_t)(buffer_get(&buffer1) * sustain));

    buffer_set(out, &buffer1);

    return out;
}


int16_t overdrive(int16_t sample, float gain, int type){
    float cutoff = 1.0f;                                                         //poziom odcięcia sygnału hardclip
    float normalized_sample = (((float)(sample)) * DB_TO_LIN(gain * 30)) / 1024; //logarytmiczne wzmocnienie sygnału i normalizacja do 1.0f
    float absolute_sample = normalized_sample;                                   // zmienna pomocnicza do obliczenia modułu


    switch (type)
    {
    case 0: // hardclip
        if (normalized_sample > cutoff) normalized_sample = cutoff;
        else if (normalized_sample < cutoff * -1) normalized_sample = cutoff * -1;
        else normalized_sample = normalized_sample;
        break;

    case 1: // softclip
        if (normalized_sample <= -1.0f) normalized_sample = -2.0f/3.0f;
        else if (normalized_sample >= 1.0f) normalized_sample = 2.0f/3.0f;
        else normalized_sample = normalized_sample - (powf(normalized_sample, 3.0f)/3.0f);
        break;

    case 2: // tangens hiperboliczny
        normalized_sample = tanhf(normalized_sample);
        break;

    case 3: // arcus tangens
        normalized_sample = atanf(normalized_sample);
        break;

    case 4: // aproksymacja tangensa
        if (normalized_sample < 0) absolute_sample = normalized_sample * -1.0f;

        normalized_sample = normalized_sample / powf((1.0f + powf(absolute_sample, 2.5f)), (1.0f/2.5f));
        break;
    
    default:
        normalized_sample = 0;
        break;
    }

    return (int16_t)(normalized_sample * POW12);
}




int16_t overdrive_echo(int16_t in, float gain, int type, uint16_t length, float sustain){
    int16_t out;
    set_buffer_delay(length, &buffer1);

    in = overdrive(in, gain, type);

    out = in + FIR_f((int16_t)(buffer_get(&buffer1) * sustain));

    buffer_set(out, &buffer1);

    return out;

}


float delta;
float sine(float frequency){

    delta += 2*M_PI / (SAMPLERATE / frequency);

    if (delta >= 2*M_PI) delta = 0;

    // printf("%i\n", out);

    return sinf(delta);
}


int16_t tremolo(int16_t sample_in){
    
    int16_t sample_out = sample_in * ( 1 + (sine(10 * pot2) * pot3));

    // printf("%i\n", sample_out);

    return sample_out;
}



uint16_t delay_counter;
uint16_t samples_counter;
uint16_t min_delay = 100;
uint16_t max_delay = 500;
uint16_t change_rate;
uint16_t control;


int16_t flanger(int16_t in){
    int16_t out;
    change_rate = 300 * pot2;
    set_buffer_delay( delay_counter, &buffer1);

    out = in + (int16_t)(buffer_get(&buffer1) * pot3);

    buffer_set(out, &buffer1);


    if (samples_counter >= change_rate) {
        delay_counter += control;
        samples_counter = 0;
    }
    else {
        samples_counter++;
    }


    if (delay_counter <= min_delay){
        control = 1;
    }
    if (delay_counter >= max_delay){
        control = -1;
    }

    // printf("%i\n", delay_counter);

    return out;
}