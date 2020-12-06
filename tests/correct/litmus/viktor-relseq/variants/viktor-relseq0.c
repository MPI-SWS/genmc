#include <pthread.h>
#include <stdatomic.h>

#include "../viktor-relseq.c"

int main()
{
        pthread_t R0, R1, R2, R3;

        pthread_create(&R0, NULL, threadR, NULL);
        pthread_create(&R1, NULL, threadRa, NULL);
        pthread_create(&R2, NULL, threadRr, NULL);
        pthread_create(&R3, NULL, threadRs, NULL);

        return 0;
}
