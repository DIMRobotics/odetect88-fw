#include <odetect/sonar.h>

#ifdef CONFIG_ODETECT_PCI0
        #define PCI0 1
#else
        #define PCI0 0
#endif

#ifdef CONFIG_ODETECT_PCI1
        #define PCI1 1
#else
        #define PCI1 0
#endif

#ifdef CONFIG_ODETECT_PCI2
        #define PCI2 1
#else
        #define PCI2 0
#endif

#ifdef CONFIG_ODETECT_PCI3
        #define PCI3 1
#else
        #define PCI3 0
#endif

#ifdef CONFIG_ODETECT_PCI4
        #define PCI4 1
#else
        #define PCI4 0
#endif

#ifdef CONFIG_ODETECT_PCI5
        #define PCI5 1
#else
        #define PCI5 0
#endif

#ifdef CONFIG_ODETECT_PCI6
        #define PCI6 1
#else
        #define PCI6 0
#endif

#ifdef CONFIG_ODETECT_PCI7
        #define PCI7 1
#else
        #define PCI7 0
#endif

static volatile enum {
        MODE_IDLE = 0,
        MODE_START,
        MODE_MEASURE
} tim_mode;

static volatile uint16_t measures[CONFIG_ODETECT_NUM_SONAR];

ANTARES_INIT_LOW(sonar_init)
{
        GPIO_INIT_OUT(GPD5);
        GPIO_INIT_OUT(GPC7);
        /* 1. init counting unit */
        /* use Timer/Counter 3 for this feature */
        /* normal mode, OCRA compare */
        TCCR3A = 0;
        //TCCR3B = (1<<CS31)|(1<<CS30); /* prescaler 64 */
        TIMSK3 = (1<<OCIE3A);

        /* 2. init external interrupts unit */
        DDRB = ~((PCI0 << 0)|(PCI1 << 1)|(PCI2 << 2)|(PCI3 << 3)|(PCI4 << 4)|(PCI5 << 5)|(PCI6 << 6)|(PCI7 << 7));
        PCMSK0 = (PCI0 << 0)|(PCI1 << 1)|(PCI2 << 2)|(PCI3 << 3)|(PCI4 << 4)|(PCI5 << 5)|(PCI6 << 6)|(PCI7 << 7);
        //PCICR |= (1 << PCIE0); /* enbale interrupts */
        
        #ifdef CONFIG_ODETECT_DEBUG
        printk("Sonars ready\n");
        #endif
}

static inline void sonars_listen(void)
{
        GPIO_WRITE_LOW(GPD5);
        for (uint8_t i=0; i<CONFIG_ODETECT_NUM_SONAR; i++)
                measures[i] = 65535; /* maximum range */
        PCICR |= (1 << PCIE0); /* enable pin change interrupt */
}

static inline void sonars_stop(void)
{
        PCICR &= ~(1 << PCIE0);
        TCCR3B &= ~(7 << CS30);
}

void sonar_start(void)
{
        /* 1. Run start impulse */
        /* After 3 timer ticks - drop impulse (on A compare channel) */
        PCMSK0 = (PCI0 << 0)|(PCI1 << 1)|(PCI2 << 2)|(PCI3 << 3)|(PCI4 << 4)|(PCI5 << 5)|(PCI6 << 6)|(PCI7 << 7);
        OCR3A = 5;
        TCNT3 = 0;
        TCCR3B = (1<<CS31)|(1<<CS30); /* run timer at prescaler 64 */
        GPIO_WRITE_HIGH(GPD5);
}

uint8_t sonar_scale(uint16_t value)
{
        return (value << 1) / 29;
}

uint8_t sonar_is_ready(void)
{
        return !(TCCR3B & (1<<CS30)); /* if timer is running, we are busy */
}

uint16_t *sonar_get(void)
{
        return measures;
}

ISR(TIMER3_COMPA_vect)
{
        if (OCR3A == 5) { /* end of start pulse */
                OCR3A = 15000; /* end of full measure cycle */
                sonars_listen();
        } else if (OCR3A == 15000) { /* end of measure cycle */
                sonars_stop();
        }
}

ISR(PCINT0_vect)
{
        static volatile uint8_t old_state = 0;
        
        uint8_t port = PINB;
        uint8_t delta = port ^ old_state;
        
        for (uint8_t i=0; delta != 0; i++, delta >>= 1) {
                if (delta & 1) {
                        if (port & (1<<i)) { /* start of pulse */
                                measures[i] = TCNT3;
                        } else { /* end of pulse */
                                measures[i] = TCNT3 - measures[i];
                                PCMSK0 &= ~(1<<i); /* disable this interrupt */
                        }
                }
        }
        
        old_state = port;
}
