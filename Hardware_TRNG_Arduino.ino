// --- Arduino Uno Pin Definitions ---
#define NOISE_PIN     A0   // Analog In: Transistor 2 Collector
#define FF_DATA_PIN    2   // Digital Out: 7474 Pin 2 (1D)
#define FF_CLK_PIN     3   // Digital Out: 7474 Pin 3 (1CLK)
#define FF_Q_PIN       4   // Digital In:  7474 Pin 5 (1Q)
#define LED_PIN       13   // Arduino Uno Built-in LED

const int BATCH_SIZE = 500; // Batch size for statistical evaluation

// 7474 Flip-Flop bit latching
int latchFlipFlop(int bitIn) {
  digitalWrite(FF_DATA_PIN, bitIn ? HIGH : LOW);
  delayMicroseconds(2);

  digitalWrite(FF_CLK_PIN, HIGH);
  delayMicroseconds(2);

  int latchedBit = digitalRead(FF_Q_PIN);

  digitalWrite(FF_CLK_PIN, LOW);
  return latchedBit;
}

// Von Neumann de-biasing algorithm
int getUnbiasedRandomBit() {
  while (true) {
    int raw1 = analogRead(NOISE_PIN) & 0x01;
    int bit1 = latchFlipFlop(raw1);
    delayMicroseconds(50);

    int raw2 = analogRead(NOISE_PIN) & 0x01;
    int bit2 = latchFlipFlop(raw2);
    delayMicroseconds(50);

    if (bit1 == 0 && bit2 == 1) return 0;
    if (bit1 == 1 && bit2 == 0) return 1;
  }
}

// Shannon Entropy: H(X) = -sum(P(x) * log2(P(x)))
float calculateEntropy(int ones, int zeros, int total) {
  if (ones == 0 || zeros == 0) return 0.0;
  float p0 = (float)zeros / (float)total;
  float p1 = (float)ones / (float)total;
  return -(p0 * (log(p0) / log(2.0)) + p1 * (log(p1) / log(2.0)));
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(FF_DATA_PIN, OUTPUT);
  pinMode(FF_CLK_PIN, OUTPUT);
  pinMode(FF_Q_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(FF_CLK_PIN, LOW);
  digitalWrite(FF_DATA_PIN, LOW);

  Serial.println(F("\n=============================================="));
  Serial.println(F("  Hardware TRNG (Arduino Uno + 5V 74LS74)     "));
  Serial.println(F("==============================================\n"));
}

void loop() {
  int onesCount = 0;
  int zerosCount = 0;
  uint32_t randomWord = 0;

  digitalWrite(LED_PIN, HIGH);
  for (int i = 0; i < BATCH_SIZE; i++) {
    int bit = getUnbiasedRandomBit();
    if (bit == 1) onesCount++;
    else zerosCount++;

    if (i < 32) {
      randomWord = (randomWord << 1) | bit;
    }
  }
  digitalWrite(LED_PIN, LOW);

  float entropy = calculateEntropy(onesCount, zerosCount, BATCH_SIZE);
  float biasOnes = ((float)onesCount / BATCH_SIZE) * 100.0;

  Serial.print(F("[Entropy Batch] 1s: "));
  Serial.print(onesCount);
  Serial.print(F(" ("));
  Serial.print(biasOnes, 1);
  Serial.print(F("%) | 0s: "));
  Serial.print(zerosCount);
  Serial.print(F(" | Shannon Entropy: "));
  Serial.print(entropy, 4);
  Serial.println(F(" / 1.0000"));

  Serial.print(F("  -> 32-bit Random Hex Seed: 0x"));
  if (randomWord < 0x10000000) Serial.print(F("0"));
  Serial.println(randomWord, HEX);
  Serial.println(F("----------------------------------------------"));

  delay(1000);
}
