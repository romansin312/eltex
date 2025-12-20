all: receiver sender sigint_blocker sigwait_loop

receiver: receiver.c
	gcc -o receiver receiver.c

sender: sender.c
	gcc -o sender sender.c

sigint_blocker: sigint_blocker.c
	gcc -o sigint_blocker sigint_blocker.c

sigwait_loop: sigwait_loop.c
	gcc -o sigwait_loop sigwait_loop.c

clean:
	rm -f receiver sender

.PHONY: all clean