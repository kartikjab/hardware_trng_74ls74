
#define NOISE_PIN     A0
#define FF_DATA_PIN    2
#define FF_CLK_PIN     3
#define FF_Q_PIN       4

// 74LS74 flip-flop se 1-bit latch aur read karna
int latchFlipFlop(int bitIn) {
  digitalWrite(FF_DATA_PIN, bitIn ? HIGH : LOW);
  delayMicroseconds(2);

  digitalWrite(FF_CLK_PIN, HIGH);
  delayMicroseconds(2);

  int latchedBit = digitalRead(FF_Q_PIN);

  digitalWrite(FF_CLK_PIN, LOW);
  return latchedBit;
}

// Von Neumann de-biasing (00 aur 11 ko discard karke 50/50 balanced bit lena)
int getUnbiasedRandomBit() {
  while (true) {
    int bit1 = latchFlipFlop(analogRead(NOISE_PIN) & 1);
    delayMicroseconds(30);

    int bit2 = latchFlipFlop(analogRead(NOISE_PIN) & 1);
    delayMicroseconds(30);

    if (bit1 == 0 && bit2 == 1) return 0;
    if (bit1 == 1 && bit2 == 0) return 1;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(FF_DATA_PIN, OUTPUT);
  pinMode(FF_CLK_PIN, OUTPUT);
  pinMode(FF_Q_PIN, INPUT);

  digitalWrite(FF_CLK_PIN, LOW);
  digitalWrite(FF_DATA_PIN, LOW);

  Serial.println(F("\n=============================================="));
  Serial.println(F("       Hardware TRNG (32-Bit Hex Output)      "));
  Serial.println(F("==============================================\n"));
}

void loop() {
  uint32_t randomWord = 0;

  // 32 unbiased physical bits ikkattha karke 32-bit number banate hain
  for (int i = 0; i < 32; i++) {
    int bit = getUnbiasedRandomBit();
    randomWord = (randomWord << 1) | bit;
  }

  // Serial Monitor par seedha Hexadecimal format mein print
  Serial.print(F("Random Hex Seed: 0x"));
  if (randomWord < 0x10000000) Serial.print(F("0")); // Leading zero formatting
  Serial.print(randomWord, HEX);

  // Saath mein decimal integer value bhi print kar dete hain
  Serial.print(F("  |  Dec: "));
  Serial.println(randomWord);

  delay(400);
}
