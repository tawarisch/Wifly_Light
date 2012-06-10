//Nils Wei� 
//08.05.2012
//Compiler CC5x 

void timer_init()
{
	T2CON = 0b01111101;
	TMR2IE = 1;
	
	T4CON = 0b01111111;
	TMR4IE = 1;
	
	PR4 = 0xff;
}

void timer_set_for_fade(char value)
{
	PR2 = value;
}
