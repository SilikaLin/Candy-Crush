#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_W 480
#define SCREEN_H 640
#define FPS 1
#define INFO 64

#define N_COLS 8
#define N_LINHAS 9
#define NUM_TYPES 4
#define MARGIN 5

typedef struct Candy {
	int type;
	int active;
	int desloca_lin;
	int desloca_col;
	ALLEGRO_COLOR cor;
} Candy;

Candy M[N_LINHAS][N_COLS];
ALLEGRO_COLOR cores[NUM_TYPES+1];

const float COL_W = (float)SCREEN_W/N_COLS;
const float LIN_W = (float)(SCREEN_H-INFO)/N_LINHAS;

int score=0, plays=13;
char my_score[100], my_plays[100];

ALLEGRO_FONT *size_f;   

int newRecord(int score, int *record) {
	FILE *arq = fopen("recorde.txt", "r");
	*record = -1;
	if(arq != NULL) {
		fscanf(arq, "%d", record);
		fclose(arq);
	}
	if(*record < score ) {
		arq = fopen("recorde.txt", "w");
		fprintf(arq, "%d", score);
		fclose(arq);
		return 1;
	}
	return 0;	
}

int generateRandomCandy() {
	return rand()%NUM_TYPES + 1;
}

void imprimeMatriz() {
	printf("\n");
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			printf("%d (%d,%d) ", M[i][j].type, M[i][j].desloca_lin, M[i][j].active);
		}
		printf("\n");
	}
}	

void completaMatriz() {
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			if(M[i][j].type == 0) {
				M[i][j].type = generateRandomCandy();
				M[i][j].desloca_col = 0;
				M[i][j].desloca_lin = 0;
				M[i][j].active = 1;		
				M[i][j].cor = cores[M[i][j].type];	
			}
		}
	}
}

void initGame() {
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			M[i][j].type = generateRandomCandy();
			M[i][j].desloca_col = 0;
			M[i][j].desloca_lin = 0;
			M[i][j].active = 1;
			M[i][j].cor = cores[M[i][j].type];
			printf("%d ", M[i][j].type);
		}
		printf("\n");
	}
}
//tempo que os doces levam para sumir
void pausa(ALLEGRO_TIMER *timer) {
	al_stop_timer(timer);
	al_rest(0.5);
	al_start_timer(timer);
}

void draw_candy(int lin, int col) {

	int cell_x = COL_W*col;
	int cell_y = INFO + LIN_W*lin;

	if(M[lin][col].type == 1) {
		al_draw_filled_triangle(cell_x+MARGIN, cell_y + LIN_W - MARGIN,
		                        cell_x + COL_W - MARGIN, cell_y + LIN_W - MARGIN,
		                        cell_x + COL_W/2, cell_y+MARGIN,
		                        M[lin][col].cor);
	} 
	else if(M[lin][col].type == 2) {
		al_draw_filled_rectangle(cell_x+2*MARGIN, cell_y+2*MARGIN,
		                         cell_x-2*MARGIN+COL_W, cell_y-2*MARGIN+LIN_W,
		                         M[lin][col].cor);

	} 
	else if(M[lin][col].type == 3) {
		al_draw_filled_rounded_rectangle(cell_x+MARGIN, cell_y+MARGIN,
		                                 cell_x-MARGIN+COL_W, cell_y-MARGIN+LIN_W,
		                                 COL_W/3, LIN_W/3, M[lin][col].cor);

	} 	
	else if(M[lin][col].type == 4) {
		al_draw_filled_ellipse(cell_x+COL_W/2, cell_y+LIN_W/2,
		                       COL_W/2-MARGIN, LIN_W/2-MARGIN,
		                       M[lin][col].cor);
	}

}

void draw_scenario(ALLEGRO_DISPLAY *display) {
	
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0); 
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);   
	
	//SCORE
	sprintf(my_score, "score: %d", score);
	al_draw_text(size_f, al_map_rgb(255, 255, 255), SCREEN_W - 200, INFO/4, 0, my_score); 
	//PLAYS
	sprintf(my_plays, "jogadas: %d", plays);
	al_draw_text(size_f, al_map_rgb(255, 255, 255), 10, INFO/4, 0, my_plays);   

	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			draw_candy(i, j);
		}
	}       

}

int clearSequence(int li, int lf, int ci, int cf) {
	int i, j, count=0;
	for(i=li; i<=lf; i++) {
		for(j=ci; j<=cf; j++) {
			count++;
			M[i][j].active = 0;
			M[i][j].cor = cores[0];
		}
	}
	return count;
}

int processaMatriz() {
	//retorna a quantidade de pontos feitos
	int i, j, k, count = 0;
	int current, seq, ultimo;

	//procura na horizontal:
	for(i=0; i<N_LINHAS; i++) {
		current = M[i][0].type;
		seq = 1;
		for(j=1; j<N_COLS; j++) {
			if(current == M[i][j].type && current > 0) {
				seq++;
				if(j == N_COLS-1 && seq >=3)
					count += clearSequence(i, i, j-seq+1, N_COLS-1);
			}
			else {
				if(seq >= 3) {
					count += clearSequence(i, i, j-seq, j-1);
				    //printf("\n1: (%d, %d), seq: %d, count: %d", i, j, seq, count);				
				}
				seq = 1;
				current = M[i][j].type;					
			}
		}
	}

	//procura na vertical:
	for(j=0; j<N_COLS; j++) {
		current = M[0][j].type;
		seq = 1;
		for(i=1; i<N_LINHAS; i++) {
			if(current == M[i][j].type && current > 0) {
				seq++;
				if(i == N_LINHAS-1 && seq >=3) 
					count += clearSequence(i-seq+1, N_LINHAS-1, j, j);
			}
			else {
				if(seq >= 3) {
					count += clearSequence(i-seq, i-1, j, j);
					//printf("\n2: (%d, %d), seq: %d, count: %d", i, j, seq, count);					
				}
				seq = 1;
				current = M[i][j].type;	

			}
		}
	}

	return count;

}

void atualizaDesloca() {
	int i, j, desloca;

	for(j=0; j<N_COLS; j++) {
		desloca = 0;
		for(i=N_LINHAS-1; i>=0; i--) {
			M[i][j].desloca_lin = desloca;
			if(M[i][j].active == 0) {
				M[i][j].type = 0;
				desloca++;
			}
		}
	}
}

void atualizaMatriz() {
	int i, j, desloca;

	for(j=0; j<N_COLS; j++) {
		for(i=N_LINHAS-1; i>=0; i--) {
			desloca = M[i][j].desloca_lin;
			if(desloca > 0) {
				M[i+desloca][j].type = M[i][j].type;
				M[i+desloca][j].active = M[i][j].active;
				M[i+desloca][j].cor = M[i][j].cor;
				M[i][j].type = 0;
				M[i][j].active = 0;
				M[i][j].desloca_lin = 0;
			}
		}
	}
}

void getCell(int x, int y, int *lin, int *col) {
	*lin = (y-INFO)/LIN_W;
	*col = x/COL_W;
}

int distancia(int lin1, int col1, int lin2, int col2) {
	return sqrt(pow(lin1-lin2, 2) + pow(col1-col2, 2));
}

int diagonal(int lin1, int col1, int lin2, int col2) {
				if(lin1!= lin2 && col1!=col2) 
					return 1;
				else
					return 0;
}

//faz as trocas
void swap(int lin1, int col1, int lin2, int col2) {
	Candy aux = M[lin1][col1];
	M[lin1][col1] = M[lin2][col2];
	M[lin2][col2] = aux;
	plays--;
}

int main(int argc, char **argv){

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_SAMPLE *music=NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	//----------------------- rotinas de inicializacao ---------------------------------------
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
		return -1;
	}

	timer = al_create_timer(0.5 / FPS);
	if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

  if(!al_install_audio()){
      fprintf(stderr, "failed to initialize audio!\n");
      return -1;
   }

   if(!al_init_acodec_addon()){
      fprintf(stderr, "failed to initialize audio codecs!\n");
      return -1;
   }
	
   if (!al_reserve_samples(1)){
      fprintf(stderr, "failed to reserve samples!\n");
      return -1;
   }

	if(!al_install_mouse())
		fprintf(stderr, "failed to initialize mouse!\n");   


	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
	al_init_ttf_addon();

	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
	size_f = al_load_font("arial.ttf", 32, 1);   	

	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}
	al_install_keyboard();
   //registra na fila de eventos que eu quero identificar quando a tela foi alterada
	al_register_event_source(event_queue, al_get_display_event_source(display));
   //registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra o teclado na fila de eventos:
	al_register_event_source(event_queue, al_get_keyboard_event_source());   
	//registra mouse na fila de eventos:
	al_register_event_source(event_queue, al_get_mouse_event_source());    
   //inicia o temporizador
	al_start_timer(timer);

	cores[0] = al_map_rgb(255,255,255);
	cores[1] = al_map_rgb(153, 51, 153);
	cores[2] = al_map_rgb(252, 15, 192);
	cores[3] = al_map_rgb(0,128,0);
	cores[4] = al_map_rgb(25,140,255);

	//----------------------- fim das rotinas de inicializacao ---------------------------------------

	music = al_load_sample( "alien.wav" );
	al_play_sample(music, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_LOOP,NULL);

	srand(2);
	initGame();
	int n_zeros = processaMatriz();
	while(n_zeros > 0) {
		do {
			atualizaDesloca();
			//imprimeMatriz();
			atualizaMatriz();
			//imprimeMatriz();			
		} while(processaMatriz());
		completaMatriz();
		//imprimeMatriz();
		n_zeros = processaMatriz();
		//imprimeMatriz();
	} 

	draw_scenario(display);
	al_flip_display();	

	int pontos, playing = 1, col_src, lin_src, col_dst, lin_dst, flag_animation=0;
	//enquanto playing for verdadeiro, faca:
	while(playing) {
		ALLEGRO_EVENT ev;
	  //espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				playing = 0;
			}

		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && !flag_animation) {
			getCell(ev.mouse.x, ev.mouse.y, &lin_src, &col_src);
		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && !flag_animation) {
			getCell(ev.mouse.x, ev.mouse.y, &lin_dst, &col_dst);
			if(distancia(lin_src, col_src, lin_dst, col_dst) == 1 && M[lin_src][col_src].type && M[lin_dst][col_dst].type ||
			distancia(lin_src, col_src, lin_dst, col_dst) == 2 && M[lin_src][col_src].type && M[lin_dst][col_dst].type ||
			distancia(lin_src, col_src, lin_dst, col_dst) == 3 && M[lin_src][col_src].type && M[lin_dst][col_dst].type &&
			diagonal(lin_src, col_src, lin_dst, col_dst) == 0) {
				swap(lin_src, col_src, lin_dst, col_dst);
				//imprimeMatriz();
				flag_animation = 1; //nao permite que o usuario faca outro comando enquanto a animacao ocorre
			}
		}		
	    //se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		else if(ev.type == ALLEGRO_EVENT_TIMER) {
			pontos = processaMatriz();
		
			while(pontos > 0) {
				//score+=pow(2,pontos);
			    draw_scenario(display);
				al_flip_display();
				pausa(timer);					
				//imprimeMatriz();
				atualizaDesloca();
				//imprimeMatriz();
				atualizaMatriz();
				//imprimeMatriz();
				score+=pow(2,pontos);
				pontos = processaMatriz();
				//printf("\n%d ", pontos);
			}

		    //reinicializo a tela
		    draw_scenario(display);
			al_flip_display();		
			
			if(plays == 0)
				playing = 0;
			flag_animation = 0;
		}
	    //se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
	} 

	al_rest(1);

	int record;
	//colore toda a tela de preto
	al_clear_to_color(al_map_rgb(230,240,250));
	sprintf(my_score, "Score: %d", score);
	al_draw_text(size_f, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2, 0, my_score);
	if(newRecord(score, &record)) {
		al_draw_text(size_f, al_map_rgb(200, 20, 30), SCREEN_W/3, 100+SCREEN_H/2, 0, "NEW RECORD!");
	}
	else {
		sprintf(my_score, "Record: %d", record);
		al_draw_text(size_f, al_map_rgb(0, 200, 30), SCREEN_W/3, 100+SCREEN_H/2, 0, my_score);
	}
	//reinicializa a tela
	al_flip_display();	
	al_rest(2);	


	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}