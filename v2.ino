#define PRAG        400
#define VITEZA_BAZA 75
#define KP          1.0f
#define TIMP_CAUTARE 900

int ultimaDirectie = 0;
unsigned long timpPierdereLinie = 0;
bool cautaLinia = false;

void setup() {
  DDRD |= (1<<3) | (1<<5) | (1<<6);
  DDRB |= (1<<1);

  TCCR2A |= (1<<COM2B1) | (1<<WGM20) | (1<<WGM21);
  TCCR2B |= (1<<CS21);
  OCR2B = 0;

  TCCR1A |= (1<<COM1A1) | (1<<WGM10);
  TCCR1B |= (1<<WGM12) | (1<<CS11);
  OCR1A = 0;

  PORTD &= ~((1<<5) | (1<<3) | (1<<6));
  PORTB &= ~(1<<1);
}

void setMotorStanga(int v) {
  v = constrain(v, -255, 255);
  if (v >= 0) { PORTD |=  (1<<5); OCR2B = (uint8_t)v;  }
  else        { PORTD &= ~(1<<5); OCR2B = (uint8_t)-v; }
}

void setMotorDreapta(int v) {
  v = constrain(v, -255, 255);
  if (v >= 0) { PORTB &= ~(1<<1); OCR1A = (uint8_t)v;  }
  else        { PORTB |=  (1<<1); OCR1A = (uint8_t)-v; }
}

void stopMotoare() {
  OCR2B = 0; OCR1A = 0;
  PORTD &= ~((1<<5) | (1<<3) | (1<<6));
  PORTB &= ~(1<<1);
}

uint8_t citireSenzori() {
  uint8_t rezultat = 0;
  for (int i = 0; i < 6; i++) {
    //if (i == 3) continue; 
    if (analogRead(i) > PRAG)
      rezultat |= (1 << i);
  }
  return rezultat;
}

int calculeazaPozitie(uint8_t senzori) {

  const int greutati[6] = {250, 150, 50, -50, -150, -250};
  int pozitie = 0, total = 0;
  for (int i = 0; i < 6; i++) {
    if ((senzori >> i) & 1) {
      pozitie += greutati[i];
      total++;
    }
  }
  if (total == 0) return 0;
  return pozitie / total;
}

void loop() {
  uint8_t senzori = citireSenzori();

  if (senzori != 0) {
    cautaLinia = false;
    int pozitie = calculeazaPozitie(senzori);


    if      (pozitie >  30) ultimaDirectie =  1;
    else if (pozitie < -30) ultimaDirectie = -1;

    int corectie = (int)(pozitie * KP);

    setMotorStanga (constrain(VITEZA_BAZA + corectie, -100, 255));
    setMotorDreapta(constrain(VITEZA_BAZA - corectie, -100, 255));
  }
  else {
    if (!cautaLinia) {
      timpPierdereLinie = millis();
      cautaLinia = true;
    }

    unsigned long durata = millis() - timpPierdereLinie;

    if (durata < TIMP_CAUTARE) {
      if (ultimaDirectie ==  1) {
        setMotorStanga(180); setMotorDreapta(-180); 
      } else if (ultimaDirectie == -1) {
        setMotorStanga(-180); setMotorDreapta(180); 
      }
    } else {
      stopMotoare();
    }
  }
}