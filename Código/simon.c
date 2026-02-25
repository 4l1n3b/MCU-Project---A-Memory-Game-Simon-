/**
*@author Aline Crislainy Barbosa da Silva Sousa; Adeilton Juscelino da Silva
*@details O código a seguir refere-se a uma versão do Simon, ou no Brasil conhecido
como Genius, um famoso jogo de memória entre as décadas de 1980 e 1990.
*/
/**
*@details Inclusao das bibliotecas 16F877A.h, biblioteca do PIC16F877A, e stdlib.h,
para utilizacao da funcao rand(). Adicionando a diretiva #use delay, para a utilizacao
da funcao delay_ms(), e a diretiva #bit TMR1IF = 0x0c.0 para a utilizacao da flag de 
estouro do timer1
*/
#include <16F877A.h>
#include <stdlib.h>
#use delay(clock = 20000000)
#bit TMR1IF = 0x0c.0
//#USE FIXED_IO (a_outputs=PIN_A4)
//#USE FIXED_IO (d_outputs=PIN_D7)
//===========DEFINIÇÕES P/ LCD ================
#define LCD_ENABLE_PIN PIN_B0
#define LCD_RS_PIN PIN_B1
#define LCD_RW_PIN PIN_B2
#define LCD_DATA4 PIN_B4
#define LCD_DATA5 PIN_B5
#define LCD_DATA6 PIN_B6
#define LCD_DATA7 PIN_B7
#include "lcd.c"

//BOTOES
#define B1 PIN_C0
#define B2 PIN_C1
#define B3 PIN_C2
#define B4 PIN_C3

//LEDS
#define BLUE PIN_A0
#define YELLOW PIN_A1
#define RED PIN_A2
#define GREEN PIN_A3

//BUZZER
#define BUZZER_PIN PIN_D7

//START
#define START_PIN PIN_C5

//===Variaveis de utilizacao no código===
int seq[32];
int player_seq[32];
int nivel = 0;
int start = 0;
int acertou = 0;
int errou = 0;
int levelplayer = 0;
int estouroTempo = 0;
int display[10] = {(0b0111111), (0b0000110), (0b1011011), (0b1001111), (0b1100110), 
(0b1101101), (0b1111101), (0b0000111), (0b1111111),(0b1101111)};
//=============Buzzer====================
/**
 * @brief Gera um tom no buzzer com a frequência e duração especificadas. A funcao 
 * e utilizada para gerar as melodias utilizadas no jogo.
 * O calculo 1000000 / frequencia converte a frequencia para o periodo em microssegundos (µs). 
   Isso ocorre porque a formula para calcular o periodo, em segundos, a partir da frequencia e:
   T = 1 / f, onde T e o periodo e f e a frequencia. Multiplicamos o resultado por 1000000 
   para converter o periodo de segundos para microssegundos.
 * @param frequencia A frequência da nota em hertz.
 * @param duracao A duração da nota em milissegundos.
 */
void tone(int16 frequencia, unsigned int duracao) {
    long periodo = 1000000 / frequencia; //<Calcula o período em microssegundos
    long metPeriodo = periodo / 2; //<Metade do período para o ciclo de trabalho
    /*Dividir o período por 2 é necessário porque o som é gerado através de um ciclo 
    de trabalho, onde o sinal é alternado entre ligado e desligado em intervalos de 
    tempo iguais à metade do período.*/
    long numCiclos = frequencia * duracao / 1000; //<Calcula o número de ciclos
    
    for (long i = 0; i < numCiclos; i++) {
	    /*o laço de repeticao itera numCiclos vezes, ligando e desligando o buzzer 
	    com intervalos de tempo correspondentes ao valor da variável "metPeriodo"*/
        output_high(BUZZER_PIN);//< Liga o buzzer
        delay_us(metPeriodo);//< Aguarda metade do período
        
        output_low(BUZZER_PIN);//< Desliga o buzzer
        delay_us(metPeriodo);//< Aguarda metade do período
    }
}
//=======================================
//=============Limpa Display==============
//funcao que limpa o display
void limpa_display() {
  lcd_putc("\f");
  delay_ms(50);
}
//===============DISPLAY 7SEG====================
/**
*@brief A funcao a seguir exibe no display de 7 segmentos o nível no qual
* o jogador esta.
*/
void exibe_nivel(){
   char unidade;
   char dezena;
   
   unidade = levelplayer%10;
   dezena = levelplayer/10;
   
   output_d(display[unidade]);/*<apresenta no display de 7 seg
   o numero do nivel do jogador no dispaly de unidade*/
   delay_ms(100);
   //mantem o dizplay da dezena desligado
   output_low(PIN_C4);
   output_low(PIN_A4);
   output_low(PIN_A5);
   output_low(PIN_E0);
   output_low(PIN_E1);
   output_low(PIN_E2);
   output_low(PIN_B3);
   //se o nivel do jogador for maior ou igual a 9 liga o segundo display
   if(levelplayer>=9){
   switch(dezena){
      case 1:
         output_low(PIN_C4);
         output_high(PIN_A4);
         output_high(PIN_A5);
         output_low(PIN_E0);
         output_low(PIN_E1);
         output_low(PIN_E2);
         output_low(PIN_B3);
         delay_ms(100);
         break;
      case 2:
         output_high(PIN_C4);
         output_high(PIN_A4);
         output_low(PIN_A5);
         output_high(PIN_E0);
         output_high(PIN_E1);
         output_low(PIN_E2);
         output_high(PIN_B3);
         delay_ms(100);
         break;
       case 3:
         output_high(PIN_C4);
         output_high(PIN_A4);
         output_high(PIN_A5);
         output_high(PIN_E0);
         output_low(PIN_E1);
         output_low(PIN_E2);
         output_high(PIN_B3);
         delay_ms(100);
         break;
   }
   }
}
//=============Start game===================
/**
 * @brief Inicia o jogo Simon.
 *
 * Esta função inicia o jogo Simon, reproduzindo uma sequência musical e aguardando
 * o sinal para começar.
 */
void start_game() {
  int16 start_music[] = {659, 783, 987, 659, 783, 987, 1200, 987};
  unsigned int tempo[] = {150, 150, 150, 150, 150, 150, 300, 150};
  limpa_display();
  lcd_gotoxy(1, 1);
  printf(lcd_putc, "Welcome to Simon");
  lcd_gotoxy(2, 2);
  printf(lcd_putc, "Press Start!");
  do {
    for (int i = 0; i < 8; i++) {
      tone(start_music[i], tempo[i]); 
      delay_ms(70);
    }
    delay_ms(70);
    if (input(START_PIN) == 0) {
      start = 1;
    }
  } while (start == 0);
}
//==========================================

//================GAME OVER=================
/**
 * @brief Encerra o jogo e exibe a tela de game over.
 *
 * Esta função é chamada quando o jogo é encerrado e exibe a tela de game over
 * juntamente com o score do jogador. Também reproduz uma sequência sonora para
 * indicar o fim do jogo.
 */
void game_over() {
  unsigned int tempo[] = {100, 100, 100, 100, 100, 100, 100,
                     100, 100, 100, 100, 100, 400, 400};
  int16 errado[] = {523, 392, 330, 440, 494, 440, 415,
                    466, 415, 392, 294, 330, 392, 392};
  limpa_display();
  lcd_gotoxy(3, 1);
  printf(lcd_putc, "GAME OVER!");
  lcd_gotoxy(8, 2);
  printf(lcd_putc, "Score: %d", levelplayer);
  for (int i = 0; i < 14; i++) {
    tone(errado[i], tempo[i]);
    delay_ms(50);
  }

  delay_ms(1000);
}
//============Playseq====================
/**
 * @brief Reproduz a sequência de botões de acordo com o nível atual.
 *
 * Esta função reproduz a sequência de botões de acordo com o nível atual do jogo.
 * Cada botão é associado a uma nota musical específica e uma cor do LED.
 * A função reproduz a nota musical, acende o LED correspondente, aguarda um período
 * de tempo e, em seguida, apaga o LED.
 */
void playSeq() {
  for (int i = 0; i <= nivel; i++) {
    switch (seq[i]) {
    case 1:
      tone(262, 3000);//Nota: C4
      output_high(BLUE);
      delay_ms(1000);
      output_low(BLUE);
      break;
    case 2:
      tone(330, 3000);//Nota: E4
      output_high(GREEN);
      delay_ms(1000);
      output_low(GREEN);
      break;
    case 3:
      tone(349, 3000);//Nota: F4
      output_high(RED);
      delay_ms(1000);
      output_low(RED);
      break;
    case 4:
      tone(293, 3000);//Nota: D4
      output_high(YELLOW);
      delay_ms(1000);
      output_low(YELLOW);
      break;
    }
  }
}
//=======================================
//==============leitura de sequencia======
/**
 * @brief Lê a sequência de entrada do jogador de acordo com o nível atual.
 *
 * Esta função lê a sequência de entrada do jogador de acordo com o nível atual do jogo.
 * Ela aguarda até que o jogador pressione um dos botões correspondentes às cores.
 * Se o jogador não pressionar um botão dentro de um determinado tempo, o jogo é encerrado.
 * Se a entrada do jogador estiver correta e dentro do tempo, ela é registrada para comparação.
 */
void ler_player() {
  set_timer1(15536);// Configura o timer para temporização
  int temp = 0;// Variável de contagem para temporização
  int press = 0; // Variável para verificar se um botão foi pressionado
  for (int i = 0; i <= nivel; i++) {
    while (press == 0) {
      if (input(B1) == 0) {
        tone(262, 3000);
        output_high(BLUE);
        delay_ms(1000);
        output_low(BLUE);
        player_seq[i] = 1;// Registra a entrada do jogador como azul
        press = 1;
      }
      if (input(B2) == 0) {
        tone(330, 3000);
        output_high(GREEN);
        delay_ms(1000);
        output_low(GREEN);
        player_seq[i] = 2;// Registra a entrada do jogador como verde
        press = 1;
      }
      if (input(B3) == 0) {
        tone(349, 3000);
        output_high(RED);
        delay_ms(1000);
        output_low(RED);
        player_seq[i] = 3;// Registra a entrada do jogador como vermelho
        press = 1;
      }
      if (input(B4) == 0) {
        tone(293, 3000);
        output_high(YELLOW);
        delay_ms(1000);
        output_low(YELLOW);// Registra a entrada do jogador como amarelo
        player_seq[i] = 4;
        press = 1;
      }
    //faz a temporizacao de 5s
    if(TMR1IF == 1){
    temp++;
    set_timer1(15536);
    TMR1IF = 0;
    }
    /*se temp chegar a 62 gera a base de tempo de 5s, iniciando a condicao
    de fim de partida por demora de inicio de jogo*/
    if(temp == 62 && press == 0){
	//todas as variaveis de controle do escopo principal do código são zeradas
    acertou = 0; 
    start = 0;
    errou = 0;
    //a funcao de fim de jogo e chamada
    game_over();
    limpa_display();
    lcd_gotoxy(1, 1);
    printf(lcd_putc, "U took so long!");
    delay_ms(1000);
    //a flag de estouro no codigo e setada
    estouroTempo = 1;
    //zeramos a variavel usada para gerar a base de tempo
    temp = 0;
    //quebramos o a estrutura while de repeticao
    break;
    }
    }//fim do while
    press = 0;
    /*e se a flag de estouro estiver setada a estrutura for 
    de repeticao tambem e quebrada, retornando para o escopo 
    principal da funcao e reiniciando a partida*/
    if(estouroTempo == 1){
    break;
    }
  }
}
//========================================
//==========INICIALIZAÇÃO DO VETOR==========
/**
 * @brief Inicializa a sequência de jogo com valores padrão.
 *
 * Esta função inicializa a sequência de jogo, preenchendo todos os elementos
 * com valores padrão (0).
 */
void init_seq() {

  for (int i = 0; i < 32; i++) {
    seq[i] = 0;// Define cada elemento da sequência como 0 (valor padrão)
  }
}
//==========================================
//==========INICIALIZAÇÃO DO VETOR player==========
/**
 * @brief Inicializa a sequência de entrada do jogador.
 *
 * Esta função inicializa a sequência de entrada do jogador, definindo todos os elementos
 * do array `player_seq` como 0. Isso prepara o array para receber as entradas do jogador
 * durante o jogo.
 */
void init_seqPlayer() {

  for (int i = 0; i < 32; i++) {
    player_seq[i] = 0;// Define todas as entradas do jogador como 0
  }
}
//==========================================

//=============GERANDO SEQ==================
/**
 * @brief Gera uma sequência de cores aleatórias.
 *
 * Esta função gera uma sequência de cores aleatórias que serão usadas no jogo.
 * A sequência é armazenada no array seq[].
 */
void gerar_seq() {
  for (int i = 0; i < 32; i = i + 2) {
    seq[i] = (rand() % 4) + 1;// Gera um número aleatório entre 1 e 4 para representar as cores
  }
}
//===========================================

void main() {
  //desativa os pinos das entradas analogivas do PORTA
  setup_adc_ports(NO_ANALOGS); 
  setup_adc(ADC_OFF);
  //configuração do timer1
  setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);
  //configuração dos pinos das portas
  set_tris_a(0b00000000);
  set_tris_c(0b00101111);
  set_tris_b(0x00);
  set_tris_d(0x00);
  set_tris_e(0x00);
  int i = 0;
  //inicia o Display LCD
  lcd_init();
  while (TRUE) {
		
    if (start == 0) {
	  exibe_nivel();  
      init_seq();
      init_seqPlayer();
      gerar_seq();
      start_game();
      nivel = 0;
      levelplayer = 0;
      estouroTempo = 0;
      
    } //fim if start = 0
    if (start == 1) { 
      limpa_display();
      lcd_gotoxy(1, 1);
      printf(lcd_putc, "Showing the seq!");
      playSeq();
      // vez do computador
      if (nivel % 2 != 0) {
        limpa_display();
        lcd_gotoxy(1, 1);
        printf(lcd_putc, "Now it's Simon!");
        playSeq();
        delay_ms(1000);
      }
      //vez do jogador
      if (nivel % 2 == 0) {
	    levelplayer++;
	    exibe_nivel();
        limpa_display();
        lcd_gotoxy(2, 1);
        printf(lcd_putc, "Now it's You!");
        lcd_gotoxy(8, 2);
        printf(lcd_putc, "Level: %d", levelplayer);
        ler_player();
      	//compara as sequencias
        for (i = 0; i <= nivel; i++) {
		  if(estouroTempo == 1){
		  	break;
		  }
          if (seq[i] == player_seq[i]) {
            acertou = 1;
          }
          if (seq[i] != player_seq[i]) {
            errou = 1;
            acertou = 0;
            break;
          }
        }
        //se o jogador acertou a sequencia
        if (acertou == 1) {
          limpa_display();
          lcd_gotoxy(1, 1);
          printf(lcd_putc, "U got it right!");
          if ((nivel + 1) % 2 != 0) {
            limpa_display();
            lcd_gotoxy(1, 1);
            printf(lcd_putc, "It's ur turn!:)");
            lcd_gotoxy(1, 2);
            printf(lcd_putc, "Enter a value");
            delay_ms(1000);
            limpa_display();
            lcd_gotoxy(1, 1);
            printf(lcd_putc, "to add to the");
            lcd_gotoxy(1, 2);
            printf(lcd_putc, "sequence!");
            delay_ms(1000);
            limpa_display();
            lcd_gotoxy(1, 1);
            printf(lcd_putc, "Press one! :)");
            int press = 0;
			//a estrutura de repetição fica em loop ate que um dos botoes sejam
			//apertados 
            while (press == 0) {
	          delay_ms(300);
              if (input(B1) == 0) {
                tone(262, 3000);
                output_high(BLUE);
                delay_ms(1000);
                output_low(BLUE);
                seq[nivel + 1] = 1;
                press = 1;
              }
              if (input(B2) == 0) {
                tone(330, 3000);
                output_high(GREEN);
                delay_ms(1000);
                output_low(GREEN);
                seq[nivel + 1] = 2;
                press = 1;
              }
              if (input(B3) == 0) {
                tone(349, 3000);
                output_high(RED);
                delay_ms(1000);
                output_low(RED);
                seq[nivel + 1] = 3;
                press = 1;
              }
              if (input(B4) == 0) {
                tone(293, 3000);
                output_high(YELLOW);
                delay_ms(1000);
                output_low(YELLOW);
                seq[nivel + 1] = 4;
                press = 1;
              }
            }
            press = 0;
          }
          acertou = 0;
        }
        //se o jogador errou
        if (errou == 1) {
          acertou = 0;
          start = 0;
          errou = 0;
          game_over();
          limpa_display();
          lcd_gotoxy(1, 1);
          printf(lcd_putc, "The correct seq:");
          playSeq();
        }
       init_seqPlayer(); 
      } //fim da vez do jogador
      nivel++;
      
    }//fim if start = 1
  }//fim do while(TRUE)
}//fim do main
