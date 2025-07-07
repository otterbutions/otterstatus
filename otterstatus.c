#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>
#include <locale.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/sysctl.h>

#include <X11/Xlib.h>

// volume defines
#define MIXER_DEVICE "/dev/mixer"
#define MIXER_VOL MIXER_READ(SOUND_MIXER_VOLUME)
// the coolest macro in the game bruh
#define AVG_LR(x) ((((x) & 0xFF) + (((x) >> 8) & 0xFF)) >> 1)

Display *dpy = NULL;
struct timespec toSleep;

char buf[48];

void dateTime(void)
{
	time_t t = time(NULL);

	char result[24];
	if(!strftime(result, sizeof(result), "%m月%d日(%a) %H:%M", localtime(&t))) return;
	
	sprintf(buf, "%s %s", buf, result);
}

void volume(void)
{
	int v, afd;

	if((afd = open(MIXER_DEVICE, O_RDONLY | O_NONBLOCK)) < 0) return;

	if(ioctl(afd, MIXER_VOL, &v) < 0)
	{
		close(afd);
		return;
	}

	sprintf(buf, "%s VOL %3d%%", buf, AVG_LR(v));
	close(afd);
}

void battery(void)
{
	int result;

	size_t len = sizeof(result);
	if(sysctlbyname("hw.acpi.battery.life", &result, &len, NULL, 0) == -1) return;

	sprintf(buf, "%s BAT %3d%%", buf, result);
}

void updatestatus(void)
{
	Window win = DefaultRootWindow(dpy);
	toSleep.tv_sec = 1;
	setlocale(LC_TIME, "");

	for(;;)
	{
		volume();
		battery();
		dateTime();
		
		XStoreName(dpy, win, buf);
		XFlush(dpy);

		buf[0] = '\0';
		nanosleep(&toSleep, NULL);
	}
}

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) return 1;

	updatestatus();
	return 0;
}
