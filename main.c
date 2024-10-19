#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h> //for IOCTL defs
#include <fcntl.h>

#define I2C_FICHIER "/dev/i2c-2" // fichier Linux representant le BUS #2
#define I2C_ADRESSE 0x68 // adresse du Device I2C


int fdI2C;  // file descriptor I2C

int main()
{
	
	fdI2C = open(I2C_FICHIER, O_RDWR);
	if(fdI2C == -1)
	{
		printf("erreur: I2C initialilse 1\n");
		return -1;
	}
	if(ioctl(fdI2C, I2C_SLAVE, I2C_ADRESSE) < 0)  // I2C_SLAVE_FORCE if it is already in use by a driver!
	{
		printf("erreur: I2C initialise 2\n");
		close(fdI2C);
		return -1;
	}
	
	
	
	
	close(fdI2C);
	return 0;
}



int piloteI2C_ecritDesOctets(uint8_t *Source, uint8_t NombreDOctetsAEcrire)
{
	if(write(fdI2C, Source, NombreDOctetsAEcrire) != NombreDOctetsAEcrire)
	{
		printf("erreur: piloteI2C_ecritDesOctets\n");
		return -1;
	}
	return 0;
}

int piloteI2C_litDesOctets(uint8_t *Commande, uint8_t LongueurDeLaCommande, uint8_t *Destination, uint8_t NombreDOctetsALire)
{
    if (piloteI2C_ecritDesOctets(Commande, LongueurDeLaCommande) < 0)
    {
    	printf("erreur: piloteI2C_litDesOctets 1\n");
    	return -1;
    }
	if (read(fdI2C, Destination, NombreDOctetsALire) != NombreDOctetsALire)
	{
		printf("erreur: piloteI2C_litDesOctets 2\n");
		return -1;
	}
	return 0;
}