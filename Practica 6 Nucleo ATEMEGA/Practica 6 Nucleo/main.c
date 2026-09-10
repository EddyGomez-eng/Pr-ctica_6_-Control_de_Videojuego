/*
 * Laboratorio 6
 * Parte 2 - Botones de accion
 *
 *
 * Arriba     -> U
 * Abajo      -> D
 * Derecha    -> R
 * Izquierda  -> L
 * Accion A   -> A
 * Accion B   -> B
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


void UART_Init(void)
{
	/*
	 * 
	 * CONFIGURACION DE LA VELOCIDAD DEL UART
	 * 
	 *
	 * El ATmega328P esta trabajando con un reloj de 16 MHz.
	 *
	 * Nosotros queremos que la comunicacion UART funcione
	 * a 9600 baudios.
	 *
	 * Para lograr esa velocidad se usa el registro UBRR0.
	 *
	 * UBRR = (F_CPU / (16 * BaudRate)) - 1
	 *
	 * UBRR = (16,000,000 / (16 * 9600)) - 1
	 *
	 * UBRR ? 103
	 *
	 * Como el registro UBRR esta dividido en dos partes:
	 *
	 * UBRR0H = parte alta
	 * UBRR0L = parte baja
	 *
	 * El numero 103 cabe completamente en la parte baja,
	 * por eso dejamos la parte alta en 0.
	 */

	UBRR0H = 0;
	UBRR0L = 103;


	/*
	 * HABILITAMOS EL TRANSMISOR UART
	 *
	 * El registro UCSR0B controla varias funciones
	 * importantes del UART.
	 *
	 * TXEN0 significa:
	 *
	 * Transmitter Enable 0
	 *
	 * Es decir, habilitar el transmisor del UART0.
	 *
	 * Cuando ponemos TXEN0 en 1, el ATmega ya puede
	 * enviar datos por su pin TX.
	 *
	 */

	UCSR0B = (1 << TXEN0);


	/*
	 * CONFIGURACION DEL TAMANO DEL DATO
	 *
	 * El registro UCSR0C configura el formato
	 * de la comunicacion UART.
	 *
	 * Los bits UCSZ01 y UCSZ00 determinan
	 * cuantos bits tendra cada dato enviado.
	 *
	 * Con:
	 *
	 * UCSZ01 = 1
	 * UCSZ00 = 1
	 *
	 * configuramos el UART para trabajar
	 * con datos de 8 bits.
	 *
	 * Esto nos permite enviar caracteres como:
	 *
	 * 'U'
	 * 'D'
	 * 'R'
	 * 'L'
	 * 'A'
	 * 'B'
	 *
	 * Cada uno de esos caracteres se representa
	 * internamente como un dato de 8 bits.
	 *
	 * Ademas, como no activamos paridad ni cambiamos
	 * los bits de parada, la configuracion queda:
	 *
	 * 9600 baudios
	 * 8 bits de datos
	 * sin paridad
	 * 1 bit de parada
	 *
	 * Esto normalmente se escribe:
	 *
	 * 9600, 8-N-1
	 */

	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
/*

 * ENVIAR UN DATO POR UART
 
 */
void UART_Transmit(uint8_t dato)
{
	/*
	 * Esperamos hasta que el registro UART
	 * este disponible para enviar otro dato.
	 */
	while (!(UCSR0A & (1 << UDRE0)))
	{
	}

	/*
	 * Colocamos el dato en el registro UART.
	 */
	UDR0 = dato;
}


/*
 * REVISION DE BOTON
 *
 * Recibe:
 *
 * pin     -> boton que queremos revisar
 * comando -> caracter que enviaremos por UART
 */
void Revisar_Boton(uint8_t pin, uint8_t comando)
{
	/*
	 * Como usamos pull-up:
	 *
	 * 1 = boton libre
	 * 0 = boton presionado
	 */
	if (!(PIND & (1 << pin)))
	{
		/*
		 * Pequena espera para eliminar el rebote
		 * mecanico del boton.
		 */
		_delay_ms(25);

		/*
		 * Revisamos nuevamente para confirmar
		 * que realmente sigue presionado.
		 */
		if (!(PIND & (1 << pin)))
		{
			/*
			 * Enviamos el comando correspondiente.
			 */
			UART_Transmit(comando);

			/*
			 * Esperamos a que el usuario suelte
			 * el boton.
			 *
			 * Esto evita mandar muchas veces
			 * el mismo comando por una sola pulsacion.
			 */
			while (!(PIND & (1 << pin)))
			{
			}

			/*
			 * Pequeno antirrebote al soltar.
			 */
			_delay_ms(25);
		}
	}
}


int main(void)
{
	/*
	 * CONFIGURACION DE BOTONES
	 *
	 * PD2 hasta PD7 se configuran como entradas.
	 *
	 * En DDRD:
	 *
	 * 0 = entrada
	 * 1 = salida
	 */

	DDRD &= ~((1 << DDD2) |
	          (1 << DDD3) |
	          (1 << DDD4) |
	          (1 << DDD5) |
	          (1 << DDD6) |
	          (1 << DDD7));


	/*
	 * Activamos los pull-ups internos
	 * de PD2 hasta PD7.
	 */
	PORTD |= ((1 << PORTD2) |
	          (1 << PORTD3) |
	          (1 << PORTD4) |
	          (1 << PORTD5) |
	          (1 << PORTD6) |
	          (1 << PORTD7));


	/*
	 * Iniciamos UART.
	 */
	UART_Init();


	/*
	 * LOOP PRINCIPAL
	 */
	while (1)
	{
		/*
		 * Revisamos cada uno de los seis botones.
		 */

		Revisar_Boton(PD2, 'U');   // Arriba

		Revisar_Boton(PD3, 'D');   // Abajo

		Revisar_Boton(PD4, 'R');   // Derecha

		Revisar_Boton(PD5, 'L');   // Izquierda

		Revisar_Boton(PD6, 'A');   // Accion A

		Revisar_Boton(PD7, 'B');   // Accion B
	}
}