#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pigpio.h>

#define PORT 5555
#define BUFFER_SIZE 1024
#define SERVO_PIN 18

#define RED_PIN 27    // Replace with the correct GPIO number for the LED's red color
#define GREEN_PIN 22  // Replace with the correct GPIO number for the LED's green color
#define BLUE_PIN 23   // Replace with the correct GPIO number for the LED's blue color

#define MOTOR_PWM_PIN 16  // Replace with the correct GPIO number for the PWM signal
#define MOTOR_IN1_PIN 20  // Replace with the correct GPIO number for IN1
#define MOTOR_IN2_PIN 21 // Replace with the correct GPIO number for IN2

void motorControl(int, int);
void direction(int);
void led(int , int , int );

int main() {
    while(1) {

        if (gpioInitialise() < 0) {
            fprintf(stderr, "Impossible d'initialiser pigpio\n");
            exit(EXIT_FAILURE);
        }

        gpioSetMode(SERVO_PIN, PI_OUTPUT);
        gpioSetMode(MOTOR_PWM_PIN, PI_OUTPUT);
        gpioSetMode(MOTOR_IN1_PIN, PI_OUTPUT);
        gpioSetMode(MOTOR_IN2_PIN, PI_OUTPUT);
        gpioSetMode(RED_PIN, PI_OUTPUT);
        gpioSetMode(GREEN_PIN, PI_OUTPUT);
        gpioSetMode(BLUE_PIN, PI_OUTPUT);

        led(0,0,1);

        int serverSocket, clientSocket;
        struct sockaddr_in serverAddr, clientAddr;
        socklen_t addrSize = sizeof(struct sockaddr_in);

        if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Erreur lors de la création de la socket serveur");
            led(255,0,0);
            exit(EXIT_FAILURE);
        }

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
            perror("Erreur lors du binding de la socket serveur");
            led(255,0,0);
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, 5) < 0) {
            perror("Erreur lors de la mise en mode écoute de la socket serveur");
            led(255,0,0);
            exit(EXIT_FAILURE);
        }

        printf("En attente de connexions...\n");

        if ((clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize)) < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }

        printf("Connexion établie avec %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        char buffer[BUFFER_SIZE];
        int bytesReceived;


        led(0, 1, 0);
        do {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';

                if (buffer[0] == 'c') {
                    int value;
                    if (sscanf(buffer + 1, "%d", &value) == 1) {
                        direction(value);
                    } else {
                        printf("Erreur de conversion pour 'd'\n");
                    }
                } else if (buffer[0] == 'm') {
                    int value;
                    if (sscanf(buffer + 1, "%d", &value) == 1) {
                        int speed = (value < -255) ? -255 : ((value > 255) ? 255 : value);
                        motorControl(abs(speed), (speed >= 0) ? 1 : -1);
                    } else {
                        printf("Erreur de conversion pour 'd'\n");
                    }
                } else if (bytesReceived < 0) {
                    perror("Erreur lors de la réception du message du client");
                }
            }
        } while (bytesReceived > 0);

        close(clientSocket);
        close(serverSocket);
        motorControl(0,1);
        gpioTerminate();
        sleep(3);
    }
}
void direction(int value) {
    gpioServo(SERVO_PIN, value);
}

void motorControl(int speed, int direction) {
    if (direction >= 0) {
        gpioWrite(MOTOR_IN1_PIN, PI_HIGH);
        gpioWrite(MOTOR_IN2_PIN, PI_LOW);
    } else {
        gpioWrite(MOTOR_IN1_PIN, PI_LOW);
        gpioWrite(MOTOR_IN2_PIN, PI_HIGH);
        direction = -direction;
    }
    if (gpioPWM(MOTOR_PWM_PIN, speed) < 0) {
        printf("erreur");
    }
}

void led(int red, int green, int blue) {
    gpioWrite(RED_PIN, red * PI_ON);
    gpioWrite(GREEN_PIN, green * PI_ON);
    gpioWrite(BLUE_PIN, blue * PI_ON);
}
