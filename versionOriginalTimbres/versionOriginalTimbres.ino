/**
BIBLIOTECA RTC DS1302


GetDateTime.cpp
*
* Example of getting the date and time from the RTC.
*
* @version 1.0.1
* @author Rafa Couto <caligari@treboada.net>
* @license GNU Affero General Public License v3.0
* @see https://github.com/Treboada/Ds1302
*
*/


#include <Arduino.h>
#include <Ds1302.h>


#include <SPI.h>
#include <Ethernet.h>

#include <LiquidCrystal.h>

LiquidCrystal lcd(A1, A2, 5, 4, 3, 2);

#define PULSADOR_IZQUIERDA A5
#define PULSADOR_OK A4
#define PULSADOR_DERECHA A3

#define BUZZER A0

#define PIN_ENA 6
#define PIN_CLK 7
#define PIN_DAT 8

#define CANTIDAD_TIMBRES 12

#define RETARDO_ANTIREBOTE 100

enum { ESTADO_TIMBRE_ACTIVO,
       ESTADO_CONFIGURACION };

int estadoSistema;
int numeroTimbre;
uint16_t tiempoDesactivacion = 0;

// DS1302 RTC instance
Ds1302 rtc(PIN_ENA, PIN_CLK, PIN_DAT);

const static char* WeekDays[] = {
  "Domingo",
  "Lunes",
  "Martes",
  "Miercoles",
  "Jueves",
  "Viernes",
  "Sabado"
};


bool banderaSonarTimbre = true;


struct formatoTimbres {
  char nombre[15];
  int hora = 12;
  int minutos = 30;
  int duracion = 3;
};


struct formatoTimbres inicioTM;
struct formatoTimbres listadoTimbres[CANTIDAD_TIMBRES];
struct formatoTimbres proximoTimbre;

/*CONEXIÓN A LA RED*/


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
IPAddress ip(10, 9, 121, 6);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void presentacion() {
  lcd.clear();  //borro el lcd
                // imprimo un mensaje en el lcd
  lcd.print("   ETEC - UBA   ");
  lcd.setCursor(0, 1);  //posicion en el cursor en la columna y fila correspondiente
  lcd.print("  Villa Lugano  ");
  delay(2000);  // Wait for 1000 millisecond(s)


  lcd.clear();  //borro el lcd
                // imprimo un mensaje en el lcd
  lcd.print("Control de");
  lcd.setCursor(0, 1);  //posicion en el cursor en la columna y fila correspondiente
  lcd.print("Timbres");
  delay(2000);  // Wait for 1000 millisecond(s)


  lcd.clear();
  // imprimo un mensaje en el lcd
  lcd.print("Integrantes:    ");
  lcd.setCursor(0, 1);
  lcd.print("Nicol Coronel   ");
  delay(2000);  // Wait for 1000 millisecond(s)


  lcd.setCursor(0, 1);
  lcd.print("Kevin Mamani    ");
  //lcd_1.setCursor(0, 1);
  delay(2000);  // Wait for 1000 millisecond(s)


  lcd.clear();
  lcd.print("   6D - 2023    ");
  delay(2000);
}

void MostrarNombreTimbreCentrado(struct formatoTimbres* timbre) {
  lcd.print("<");
  if (strlen((*timbre).nombre) < 14) {
    for (int i = 1; i <= ((14 - strlen((*timbre).nombre)) / 2); i++)
      lcd.print(" ");

    if (strlen((*timbre).nombre) % 2 == 0) {
      lcd.print((*timbre).nombre);
    } else {
      lcd.print((*timbre).nombre);
      lcd.print(" ");
    }

    for (int i = 1; i <= ((14 - strlen((*timbre).nombre)) / 2); i++)
      lcd.print(" ");
  } else {
    lcd.print((*timbre).nombre);
  }
  lcd.setCursor(15, 0);
  lcd.print(">");
}

void MostrarDatosTimbre(struct formatoTimbres* timbre) {

  if ((*timbre).hora < 10)
    lcd.print("0");
  lcd.print((*timbre).hora);

  lcd.print(":");
  if ((*timbre).minutos < 10)
    lcd.print("0");
  lcd.print((*timbre).minutos);

  lcd.print(" ");
  if ((*timbre).duracion < 10)
    lcd.print("0");
  lcd.print((*timbre).duracion);
  lcd.print(" Seg");
}

void ModoficarTimbre(struct formatoTimbres* timbre) {

  bool banderaModificacion = true, banderaParpadeo = true;

  int contadorModificacion = 0, contadorMiliSegundos = 0;

  while (banderaModificacion == true) {

    lcd.setCursor(0, 0);
    lcd.print((*timbre).nombre);
    lcd.print(": ");
    lcd.setCursor(0, 1);


    if ((*timbre).hora < 10)
      lcd.print("0");
    lcd.print((*timbre).hora);
    lcd.print(":");
    if ((*timbre).minutos < 10)
      lcd.print("0");
    lcd.print((*timbre).minutos);
    lcd.print(" ");
    if ((*timbre).duracion < 10)
      lcd.print("0");
    lcd.print((*timbre).duracion);
    lcd.print(" Seg");


    if (contadorMiliSegundos >= 5) {
      contadorMiliSegundos = 0;


      banderaParpadeo = !banderaParpadeo;
    }


    if (banderaParpadeo == true) {
      lcd.setCursor(contadorModificacion, 1);
      lcd.print("  ");
    }


    if (digitalRead(PULSADOR_OK) == LOW) {




      delay(RETARDO_ANTIREBOTE);
      while (digitalRead(PULSADOR_OK) == LOW)
        ;
      delay(RETARDO_ANTIREBOTE);




      contadorModificacion += 3;


      if (contadorModificacion == 9) {
        banderaModificacion = false;
      }
    }


    switch (contadorModificacion) {


      case 0:


        if (digitalRead(PULSADOR_DERECHA) == LOW) {
          (*timbre).hora++;
          if ((*timbre).hora == 24) {
            (*timbre).hora = 0;
          }
        }


        if (digitalRead(PULSADOR_IZQUIERDA) == LOW) {
          (*timbre).hora--;
          if ((*timbre).hora == -1) {
            (*timbre).hora = 23;
          }
        }


        break;


      case 3:


        if (digitalRead(PULSADOR_DERECHA) == LOW) {
          (*timbre).minutos++;
          if ((*timbre).minutos == 60) {
            (*timbre).minutos = 0;
          }
        }


        if (digitalRead(PULSADOR_IZQUIERDA) == LOW) {
          (*timbre).minutos--;
          if ((*timbre).minutos == -1) {
            (*timbre).minutos = 59;
          }
        }


        break;


      case 6:


        if (digitalRead(PULSADOR_DERECHA) == LOW) {
          (*timbre).duracion++;
          if ((*timbre).duracion == 16) {
            (*timbre).duracion = 1;
          }
        }


        if (digitalRead(PULSADOR_IZQUIERDA) == LOW) {
          (*timbre).duracion--;
          if ((*timbre).duracion == 0) {
            (*timbre).duracion = 15;
          }
        }


        break;
    }


    delay(100);
    contadorMiliSegundos++;
  }
}

struct formatoTimbres buscarProximoTimbre(formatoTimbres listadoTimbres[]) {


  int i;


  formatoTimbres proximo;


  //int hora = 12;
  //int minutos = 25;


  Ds1302::DateTime now;
  rtc.getDateTime(&now);  //Aca estamos obteniendo la hora actual


  int diferencia, diferenciaMinima;
  bool banderaPrimerTimbrePosterior = true, banderaPrimerTimbrePAnterior = true, hayTimbrePosterior = false;


  for (i = 0; i < CANTIDAD_TIMBRES; i++) {


    diferencia = ((listadoTimbres[i].hora * 100) + listadoTimbres[i].minutos) - ((now.hour * 100) + now.minute);  //Aca los * 100 son por la nomenclatura militar


    Serial.print(listadoTimbres[i].hora);
    Serial.print(" ");
    Serial.println(diferencia);


    if (diferencia > 0) {  //Aca decimos que si la diferencia (horaTimbre - horaActual) es mayor a 0 significa que todavia hay un timbre que va a sonar.
      hayTimbrePosterior = true;
      if (banderaPrimerTimbrePosterior == true) {
        banderaPrimerTimbrePosterior = false;  //Y la variable diferenciaMinima esta declarada por que hay que comparar los resultados negativos para saber cual es el proximo timbre
        diferenciaMinima = diferencia;
        proximo = listadoTimbres[i];
      } else {


        if (diferencia < diferenciaMinima) {
          proximo = listadoTimbres[i];
        }
      }
    }
    /*
   else {  // Este else es por si diferencia es menor a 0. Porque si es asi y si todos son asi significa que ya sonaron todos los timbres.
     if (banderaPrimerTimbrePAnterior == true) {
       banderaPrimerTimbrePAnterior = false;
       diferenciaMinima = diferencia;
       proximo = listadoTimbres[i];
     } else {


       if (diferencia < diferenciaMinima) {
         proximo = listadoTimbres[i];
       }
     }
   }
   */
  }


  Serial.print("proximo: ");


  Serial.println(proximo.hora);


  return proximo;
}

void mostrarFechaYHora(Ds1302::DateTime now){
          lcd.setCursor(0, 0);
        lcd.print(WeekDays[now.dow]);
        lcd.setCursor(2, 0);
        lcd.print(" ");
        lcd.print(now.day);
        lcd.print("/");
        lcd.print(now.month);
        lcd.print("/");
        lcd.print(now.year);
        lcd.print(" ");
        if (now.hour < 10)
          lcd.print("0");
        lcd.print(now.hour);
        if ((now.second % 2) == 0)
          lcd.print(":");
        else
          lcd.print(" ");
        if (now.minute < 10)
          lcd.print("0");
        lcd.print(now.minute);

        /*
        lcd.print(":");
        if (now.second < 10)
          lcd.print("0");
        lcd.print(now.second);
        */
}

void mostrarProximoTimbre()
{
  lcd.setCursor(0, 1);
  lcd.print("Prox: ");
  if (proximoTimbre.hora < 10)
    lcd.print("0");
  lcd.print(proximoTimbre.hora);
  lcd.print(":");
  if (proximoTimbre.minutos < 10)
    lcd.print("0");
  lcd.print(proximoTimbre.minutos);
  lcd.print(" ");
  if (proximoTimbre.duracion < 10)
    lcd.print("0");
  lcd.print(proximoTimbre.duracion);
  lcd.print(" S");
}

void hacerSonarTimbre(){
  //Serial.println("Timbreeeeeeeeeeeeeeeeeeeeeeeee");
  tone(BUZZER, 440, proximoTimbre.duracion * 1000);

  lcd.clear();
  lcd.print("TIMBRE!");
  lcd.setCursor(0, 1);
  lcd.print(proximoTimbre.nombre);
  delay(proximoTimbre.duracion * 1000);
}

bool seleccionaConfiguracion() {
  return digitalRead(PULSADOR_DERECHA) == LOW && digitalRead(PULSADOR_IZQUIERDA) == LOW;
}

void mostrarMensajeConfiguracion(){
  lcd.clear();
  lcd.print("Configurar");
  lcd.setCursor(0, 1);
  lcd.print("timbres...");
  delay(2000);
  lcd.clear();
}

void setup() {
  Serial.begin(9600);
  pinMode(PULSADOR_IZQUIERDA, INPUT_PULLUP);
  pinMode(PULSADOR_OK, INPUT_PULLUP);
  pinMode(PULSADOR_DERECHA, INPUT_PULLUP);
  // initialize the RTC
  rtc.init();

  /*
// test if clock is halted and set a date-time (see example 2) to start it

if (rtc.isHalted()) {
  Serial.println("RTC is halted. Setting time...");

  Ds1302::DateTime dt = {
    .year = 17,
    .month = Ds1302::MONTH_OCT,
    .day = 3,
    .hour = 9,
    .minute = 21,
    .second = 30,
    .dow = Ds1302::DOW_TUE
  };

  rtc.setDateTime(&dt);
}
*/

  /*SETIAR LA HORA EN RTC*/

/*
Serial.println("Setting time...");

Ds1302::DateTime dt = {
 .year = 23,
 .month = 9,  //Ds1302::MONTH_AGO,
 .day = 15,
 .hour = 9,
 .minute = 19,
 .second = 0,
 .dow = 5  //Ds1302::DOW_TUE
};

rtc.setDateTime(&dt);
*/

  /*LCD*/

  delay(2000);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  //lcd.print("HOLA");

  /*Inicialización de Timbres*/

  strcpy(listadoTimbres[0].nombre, "inicioTM");
  listadoTimbres[0].hora = 7;
  listadoTimbres[0].minutos = 40;
  listadoTimbres[0].duracion = 5;

  strcpy(listadoTimbres[1].nombre, "inicioRecreoTM");
  listadoTimbres[1].hora = 10;
  listadoTimbres[1].minutos = 10;
  listadoTimbres[1].duracion = 5;

  strcpy(listadoTimbres[2].nombre, "FinRecreoTM");
  listadoTimbres[2].hora = 10;
  listadoTimbres[2].minutos = 20;
  listadoTimbres[2].duracion = 3;

  strcpy(listadoTimbres[3].nombre, "Almuerzo");
  listadoTimbres[3].hora = 12;
  listadoTimbres[3].minutos = 10;
  listadoTimbres[3].duracion = 3;

  strcpy(listadoTimbres[4].nombre, "FinAlmuerzo");
  listadoTimbres[4].hora = 13;
  listadoTimbres[4].minutos = 20;
  listadoTimbres[4].duracion = 3;

  strcpy(listadoTimbres[5].nombre, "RecreoTT");
  listadoTimbres[5].hora = 15;
  listadoTimbres[5].minutos = 10;
  listadoTimbres[5].duracion = 3;

  strcpy(listadoTimbres[6].nombre, "FinRecreoTT");
  listadoTimbres[6].hora = 15;
  listadoTimbres[6].minutos = 20;
  listadoTimbres[6].duracion = 3;

  strcpy(listadoTimbres[7].nombre, "Merienda");
  listadoTimbres[7].hora = 17;
  listadoTimbres[7].minutos = 10;
  listadoTimbres[7].duracion = 3;

  strcpy(listadoTimbres[8].nombre, "FinMerienda");
  listadoTimbres[8].hora = 17;
  listadoTimbres[8].minutos = 20;
  listadoTimbres[8].duracion = 3;

  strcpy(listadoTimbres[9].nombre, "RecreoTN");
  listadoTimbres[9].hora = 19;
  listadoTimbres[9].minutos = 10;
  listadoTimbres[9].duracion = 3;

  strcpy(listadoTimbres[10].nombre, "RecreoYSalida");
  listadoTimbres[10].hora = 19;
  listadoTimbres[10].minutos = 20;
  listadoTimbres[10].duracion = 3;

  strcpy(listadoTimbres[11].nombre, "Salida");
  listadoTimbres[11].hora = 20;
  listadoTimbres[11].minutos = 00;
  listadoTimbres[11].duracion = 3;

  estadoSistema = ESTADO_TIMBRE_ACTIVO;

  // Leo la hora actual
  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  proximoTimbre = buscarProximoTimbre(listadoTimbres);

  presentacion();

  lcd.clear();
  lcd.print("Conectando...");
  lcd.setCursor(0, 1);
  lcd.print("IP: ");

  /*ETHERNET POR DHCP*/

  /*
 // start the Ethernet connection:
 Serial.println("Initialize Ethernet with DHCP:");
 if (Ethernet.begin(mac) == 0) {
   Serial.println("Failed to configure Ethernet using DHCP");
   if (Ethernet.hardwareStatus() == EthernetNoHardware) {
     Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
   } else if (Ethernet.linkStatus() == LinkOFF) {
     Serial.println("Ethernet cable is not connected.");
   }
   // no point in carrying on, so do nothing forevermore:
   while (true) {
     delay(1);
   }
 }
 */

  /*ETHERNET IP FIJA*/

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  lcd.print(Ethernet.localIP());

  delay(3000);
}

void loop() {

  // Leo la hora actual
  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  switch (estadoSistema) {

    case ESTADO_TIMBRE_ACTIVO:

      unsigned long tiempoActual = millis();
      static uint8_t last_second = 0;

      // 1) cada cambio de segundo actualiza la pantalla
      if (last_second != now.second) {  
        last_second = now.second;

        /*IMPRIMO EN EL LCD*/
        mostrarFechaYHora(now);
        mostrarProximoTimbre();
      }

      //Serial.print("banderaSonarTimbre ");    NOSE PARA QUE ES ESTO
      //Serial.println(banderaSonarTimbre);

      if ((now.hour == proximoTimbre.hora) && (now.minute == proximoTimbre.minutos) && banderaSonarTimbre == true) {
        
        banderaSonarTimbre = false;

        proximoTimbre = buscarProximoTimbre(listadoTimbres);

        tiempoDesactivacion = millis();  // Tomo el tiempo para habilitar mas tarde el timbre.
      }

      if (millis() >= (tiempoDesactivacion + 61000) && banderaSonarTimbre == false) {  // Si pasa un poco más de un minuto del timbre lo vuelvo a habilitar
        banderaSonarTimbre = true;
      }

      //delay(100);

      if (seleccionaConfiguracion()) {
        mostrarMensajeConfiguracion();
        numeroTimbre = 0;
        estadoSistema = ESTADO_CONFIGURACION;
      }
      break;

    case ESTADO_CONFIGURACION:

      lcd.setCursor(0, 0);
      MostrarNombreTimbreCentrado(&listadoTimbres[numeroTimbre]);
      lcd.setCursor(0, 1);
      lcd.print("  ");
      MostrarDatosTimbre(&listadoTimbres[numeroTimbre]);
      lcd.print("  ");

      if (digitalRead(PULSADOR_DERECHA) == LOW) {

        delay(RETARDO_ANTIREBOTE);
        while (digitalRead(PULSADOR_DERECHA) == LOW)
          ;
        delay(RETARDO_ANTIREBOTE);

        numeroTimbre++;

        if (numeroTimbre == CANTIDAD_TIMBRES) {
          numeroTimbre = 0;
        }
      }

      if (digitalRead(PULSADOR_IZQUIERDA) == LOW) {




        delay(RETARDO_ANTIREBOTE);
        while (digitalRead(PULSADOR_IZQUIERDA) == LOW)
          ;
        delay(RETARDO_ANTIREBOTE);




        numeroTimbre--;




        if (numeroTimbre == -1) {
          numeroTimbre = CANTIDAD_TIMBRES - 1;
        }
      }


      if (digitalRead(PULSADOR_OK) == LOW) {


        delay(RETARDO_ANTIREBOTE);
        while (digitalRead(PULSADOR_OK) == LOW)
          ;
        delay(RETARDO_ANTIREBOTE);


        lcd.clear();


        ModoficarTimbre(&listadoTimbres[numeroTimbre]);


        lcd.clear();
        lcd.print("Cambio OK");
        lcd.setCursor(0, 1);
        MostrarDatosTimbre(&listadoTimbres[numeroTimbre]);
        delay(3000);
        lcd.clear();


        // Leo la hora actual
        Ds1302::DateTime now;
        rtc.getDateTime(&now);


        proximoTimbre = buscarProximoTimbre(listadoTimbres);


        estadoSistema = ESTADO_TIMBRE_ACTIVO;
      }


      break;
  }


  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");


    Ds1302::DateTime now;
    rtc.getDateTime(&now);


    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");         // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
          client.println("</head>");


          client.println("<H1 align='center'>");
          client.print("Control de timbres ETEC UBA");
          client.println("</H1>");


          client.println("<H2 align='center'>");
          client.print("Nicole Coronel y Kevin Mamani");
          client.println("</H2>");


          client.println("<H3 align='center'>");
          client.print("Hora Actual: ");


          /*IMPRIMO EN EL LCD*/
          if (now.hour < 10)
            client.print("0");
          client.print(now.hour);
          client.print(":");
          if (now.minute < 10)
            client.print("0");
          client.print(now.minute);
          client.println("</H3>");


          client.println("<H3 align='center'>");
          client.print("Proximo timbre: ");
          client.println("</H3>");

          client.println("<H3 align='center'>");
          client.print(proximoTimbre.nombre);
          client.print(" ");
          if (proximoTimbre.hora < 10)
            client.print("0");
          client.print(proximoTimbre.hora);
          client.print(":");
          if (proximoTimbre.minutos < 10)
            client.print("0");
          client.print(proximoTimbre.minutos);
          client.print(" ");
          client.print("(");
          client.print(proximoTimbre.duracion);
          client.print(" Segundos)");
          client.println("</H3>");

          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
