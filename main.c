#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h> //for IOCTL defs
#include <fcntl.h>

#define I2C_FICHIER "/dev/i2c-1" // fichier Linux representant le BUS #2
#define I2C_ADRESSE 0x29 // adresse du Device I2C MPU-9250 (motion tracking)


typedef struct
{
    uint16_t reg;  // Registre à configurer
    uint8_t valeur; // Valeur de configuration
} VL6180Xconfig;

VL6180Xconfig VL6810x_message[] = {
    {0x0207, 0x01}, {0x0208, 0x01}, {0x0133, 0x01}, 
	{0x0096, 0x00}, {0x0097, 0xFD}, {0x00e3, 0x00}, 
	{0x00e4, 0x04}, {0x00e5, 0x02}, {0x00e6, 0x01},
    {0x00e7, 0x03}, {0x00f5, 0x02}, {0x00d9, 0x05}, 
	{0x00db ,0xce}, {0x00dc, 0x03}, {0x00dd, 0xf8}, 
	{0x009f, 0x00}, {0x00a3, 0x3c}, {0x00b7, 0x00}, 
	{0x00bb, 0x3c}, {0x00b2, 0x09}, {0x00ca, 0x09},
    {0x0198, 0x01}, 
};


int initialise_VL6180X(int fd);
int lireDistance(float *Distance); 
int ecrireRegistre(int fd, uint16_t registre, uint8_t donnee); 
int lireRegistre(int fd, uint16_t registre, uint8_t *donnee); 

int main()
{
	int fdPortI2C;  // file descriptor I2C

	// Initialisation du port I2C, 
	fdPortI2C = open(I2C_FICHIER, O_RDWR); // ouverture du 'fichier', création d'un 'file descriptor' vers le port I2C
	if(fdPortI2C == -1)
	{
		printf("erreur: I2C initialisation step 1\n");
		return -1;
	}
	if(ioctl(fdPortI2C, I2C_SLAVE_FORCE, I2C_ADRESSE) < 0)  // I2C_SLAVE_FORCE if it is already in use by a driver (i2cdetect : UU)
	{
		printf("erreur: I2C initialisation step 2\n");
		close(fdPortI2C);
		return -1;
	}
	
	// Écriture et Lecture sur le port I2C
	uint8_t Source[2] = {0x00,0x00}; // registre d'ID du chip I2C
	uint8_t Destination;
	uint8_t NombreDOctetsALire = 1;
	uint8_t NombreDOctetsAEcrire = 2;


	if(write(fdPortI2C, Source, NombreDOctetsAEcrire) != NombreDOctetsAEcrire)
	{
		printf("erreur: Écriture I2C\n");
		close(fdPortI2C);
		return -1;
	}
	if (read(fdPortI2C, &Destination, NombreDOctetsALire) != NombreDOctetsALire)
	{
		printf("erreur: Lecture I2C\n");
		close(fdPortI2C);
		return -1;
	}
	printf("octets lus: %#04x\n", Destination);

	// Initialisation du capteur
    if (initialise_VL6180X(fdPortI2C) != 0) 
	{
        close(fdPortI2C);
        return -1;
    }

    // Lecture de la distance
    if (lireDistance(fdPortI2C) != 0) 
	{
        close(fdPortI2C);
        return -1;
    }




	close(fdPortI2C); /// Fermeture du 'file descriptor'
	return 0;
}


//FONCTIONS
int initialise_VL6180X(int fd)
{
    for (int i = 0; i < sizeof(VL6810x_message) / sizeof(VL6180Xconfig); i++) 
	{
        uint8_t buffer[3] = { VL6810x_message[i].reg >> 8, 
                              VL6810x_message[i].reg & 0xFF, 
                              VL6810x_message[i].valeur };
        if (write(fd, buffer, 3) != 3)  
		{
            printf("Erreur: Initialisation VL6180X\n");
            return -1;
        }
    }
    return 0;
}

int lireDistance(float *Distance)
{
	uint8_t valeur;
	if (interfaceVL6180x_ecrit(0x18, 0x01) < 0)
	{
		printf("erreur: interfaceVL6180x_litUneDistance 1\n");
		return -1;
	}
	if(interfaceVL6180x_lit(0x4F, &valeur) < 0)
	{
		printf("erreur: interfaceVL6180x_litUneDistance 2\n");
		return -1;
	}
	while((valeur & 0x7) != 4)
	{
		if (interfaceVL6180x_lit(0x4F, &valeur) < 0)
		{
			printf("erreur: interfaceVL6180x_litUneDistance 3\n");
			return -1;
		}
  	}
  	if(interfaceVL6180x_lit(0x62, &valeur) < 0)
  	{
		printf("erreur: interfaceVL6180x_litUneDistance 4\n");
    	return -1;
	}
	if (interfaceVL6180x_ecrit(0x15, 0x07) < 0)
  	{
  		printf("erreur: interfaceVL6180x_litUneDistance 5\n");
    	return -1;
	}
	*Distance = (float)valeur /10.0;
	return 0;
}

int ecrireRegistre(int fd, uint16_t registre, uint8_t donnee) 
{
	uint8_t message[3];
	message[0] = (uint8_t)(registre >> 8);
	message[1] = (uint8_t)(registre & 0xFF);
	message[2] = donnee;
	if (piloteI2C1_ecritDesOctets(message, 3) < 0)
	{
		printf("erreur: interfaceVL6180x_ecrit\n");
		return -1;
	}
	return 0;
}

int lireRegistre(int fd, uint16_t registre, uint8_t *donnee)
{
	uint8_t Commande[2];
	Commande[0] = (uint8_t)(registre >> 8);
	Commande[1] = (uint8_t)registre;
	if (piloteI2C1_litDesOctets(Commande, 2, donnee, 1) < 0)
	{
		printf("erreur: interfaceVL6180x_lit\n");
		return -1;
	}
	return 0;
}
