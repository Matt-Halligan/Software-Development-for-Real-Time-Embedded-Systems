////////////////////////////////////////////////////
// 
//  Matthew Halligan
//  Software Development for Real Time Embedded Systems, Fall 2025
//  Module 1/2 Project 1
//  Professor Doug Ferguson
// 
// Morse Code Optical Communication on Arduino Uno
// Implements [a-z][0-9][" "][^] character comms 
// using Circular Linked List datastructure to acheive 
// "Round Robin Design" and display
// 
// To use, ensure serial monitor is attached via Arduino IDE 2.3.6 set to 9600 baud
// Submit a message to Arduino Uno on COM5 with "New Line" as message terminator
// To clear the current message, enter the sentinel character "^"
// To append to the current message and restart transmission, submit the message.
// To clear the current message and begin a new message immediately submit the sentinel immediately followed by the message i.e.[^][a-z0-9]*
// 
////////////////////////////////////////////////////
int switch_state = 0;
char buffer [31];
char tempbuffer [1];
byte index = 0;
char c = ' ';
char termChar = '\n'; // newline
char resetChar = '^'; // ^
const int unit_time = 250;
struct linkedNode* head = NULL;
struct linkedNode* temp = NULL;

struct linkedNode{
  char data;
  struct linkedNode* next;
};

struct linkedNode* createNode(char* character){
  struct linkedNode* newNode = (struct linkedNode*)malloc(sizeof(struct linkedNode));
  if (!newNode) return NULL; 
  newNode->data = character;
  newNode->next = NULL;
  return newNode;
}

void insertAtEnd(struct linkedNode** head, char data)
{
    // Serial.print("Inserting at End.\n");
    struct linkedNode* newNode = createNode(data);
    if (!newNode) return;
    if (*head == NULL) {
        *head = newNode;
        newNode->next = *head;
        Serial.print("Setting new Head\n");
    }
    else {
        struct linkedNode* temp = *head;
        while (temp->next != *head) {
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
    }
    Serial.print("Successfully inserted\n");

}

void clearList(struct linkedNode** head){
  Serial.print("Clearing List\n");
  if (*head == NULL) {
    Serial.print("Null Case, Returning\n");
    return;
  } 
  struct linkedNode* cur = (*head)->next;
  while (cur != *head){
    struct linkedNode* nxt = cur->next;
    Serial.print("Freeing ");
    Serial.print((cur)->data);
    Serial.print("\n");
    free(cur);
    cur = nxt; 
  }
  free(*head);
  *head = NULL;
  Serial.print("Completed Clearing List\n");
  return;
}


void blink(){
  digitalWrite(3, HIGH);
  delay(unit_time);
  digitalWrite(3, LOW);
}

void dash(){
  digitalWrite(3, HIGH);
  delay(3*unit_time);
  digitalWrite(3, LOW);
}

void space_on_short(){
  delay(unit_time);
}

void space_on_letter(){
  delay(3*unit_time);
}

void space_on_word(){
  delay(7*unit_time);
}

void switchMorse(char data){
  switch(data){
    case ' ':
    Serial.print("[SPACE]");
    // Serial.print("\n");
    space_on_word();
    break;
    case 'a':
    Serial.print("a");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'b':
    Serial.print("b");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'c':
    Serial.print("c");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'd':
    Serial.print("d");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'e':
    Serial.print("e");
    // Serial.print("\n");
    blink();
    space_on_letter();
    break;
    case 'f':
    Serial.print("f");
    // Serial.print("\n");    
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'g':
    Serial.print("g");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'h':
    Serial.print("h");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'i':
    Serial.print("i");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'j':
    Serial.print("j");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'k':
    Serial.print("k");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'l':
    Serial.print("l");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'm':
    Serial.print("m");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'n':
    Serial.print("n");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'o':
    Serial.print("o");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'p':
    Serial.print("p");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 'q':
    Serial.print("q");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'r':
    Serial.print("r");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 's':
    Serial.print("s");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case 't':
    Serial.print("t");
    // Serial.print("\n");
    dash();
    space_on_letter();
    break;
    case 'u':
    Serial.print("u");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'v':
    Serial.print("v");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'w':
    Serial.print("w");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'x':
    Serial.print("x");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'y':
    Serial.print("y");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case 'z':
    Serial.print("z");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case '0':
    Serial.print("0");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case '1':
    Serial.print("1");
    // Serial.print("\n");
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case '2':
    Serial.print("2");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case '3':
    Serial.print("3");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case '4':
    Serial.print("4");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    dash();
    space_on_letter();
    break;
    case '5':
    Serial.print("5");
    // Serial.print("\n");
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case '6':
    Serial.print("6");
    // Serial.print("\n");
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case '7':
    Serial.print("7");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case '8':
    Serial.print("8");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    blink();
    space_on_short();
    blink();
    space_on_letter();
    break;
    case '9':
    Serial.print("9");
    // Serial.print("\n");
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_short();
    dash();
    space_on_letter();
    break;
  }
}



void processNewString(){
  for (int i=0; i<(int)sizeof(buffer); i++){

    if (buffer[i] == '\0' || buffer[i]=='\n') {
      // Serial.print("Found End Character, Returning\n");
      buffer[i] = NULL;
      return;
    }
    Serial.print("\nInserting ");
    Serial.print(buffer[i]);
    Serial.print("\n");
    insertAtEnd(&head, buffer[i]);
    temp = head;
    buffer[i] = NULL;
  }
}

void processInput(){
  while (Serial.available()){
    Serial.readBytes(tempbuffer, 1);

    if (tempbuffer[0] == resetChar){
      Serial.print("Interrupt Enterred");
      clearList(&head);
      return;
    }
    
    if (tempbuffer[0] != termChar){
      buffer[index] = tempbuffer[0];
      index = index + 1;
    }else if (tempbuffer[0] == termChar){
      buffer[index] = '\0';
      index = 0;
    }
     
  }
  if (buffer[0] != NULL || buffer[0] != '\0'){
    processNewString();
  } 
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  
  // Insertion
  // insertAtEnd(&head, 'a');
  // insertAtEnd(&head, 'b');
  // insertAtEnd(&head, 'c');
  // insertAtEnd(&head, 'd');
  // insertAtEnd(&head, 'e');
  // insertAtEnd(&head, 'f');
  // insertAtEnd(&head, 'g');
  // insertAtEnd(&head, 'h');
  // insertAtEnd(&head, 'i');
  // insertAtEnd(&head, 'j');
  // insertAtEnd(&head, 'k');
  // insertAtEnd(&head, 'l');
  // insertAtEnd(&head, 'm');
  // insertAtEnd(&head, 'n');
  // insertAtEnd(&head, 'o');
  // insertAtEnd(&head, 'p');
  // insertAtEnd(&head, 'q');
  // insertAtEnd(&head, 'r');
  // insertAtEnd(&head, 's');
  // insertAtEnd(&head, 't');
  // insertAtEnd(&head, 'u');
  // insertAtEnd(&head, 'v');
  // insertAtEnd(&head, 'w');
  // insertAtEnd(&head, 'x');
  // insertAtEnd(&head, 'y');
  // insertAtEnd(&head, 'z');
  // insertAtEnd(&head, '0');
  // insertAtEnd(&head, '1');
  // insertAtEnd(&head, '2');
  // insertAtEnd(&head, '3');
  // insertAtEnd(&head, '4');
  // insertAtEnd(&head, '5');
  // insertAtEnd(&head, '6');
  // insertAtEnd(&head, '7');
  // insertAtEnd(&head, '8');
  // insertAtEnd(&head, '9');

  //   Serial.print("Circular Linked List: ");
  delay(350);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (head == NULL) {
  //     Serial.print("NULL\n");
  // }

  temp = head;
  Serial.print("\n");
  processInput();
  do {
      
    processInput();  
    //   Serial.print(temp->data);
    //   Serial.print("\n");
    if (head != NULL){
      
      switchMorse(temp->data);
      temp = temp->next;
    } else{
      break;
    }
      
  } while (temp != head);
  // Serial.print("HEAD\n");
delay(500);
}