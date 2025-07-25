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

#include <X11/XKBlib.h>
#include <X11/Xlib.h>

#define MAX_BUF_SIZE 47
// volume defines
#define MIXER_DEVICE "/dev/mixer"
#define MIXER_VOL MIXER_READ(SOUND_MIXER_VOLUME)
// the coolest macro in the game bruh
#define KEYMAP_GROUP XGetAtomName(dpy, desc->names->groups[state.group])

Display *dpy;
struct timespec toSleep;

char buf[MAX_BUF_SIZE+1];

void dateTime(void)
{
	time_t t = time(NULL);

	char result[24];
	if(!strftime(result, sizeof(result), "%m月%d日(%a) %H:%M", localtime(&t))) return;
	
	snprintf(buf, MAX_BUF_SIZE, "%s %s", buf, result);
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

	snprintf(buf, MAX_BUF_SIZE, "%s VOL %3d%%", buf, v & 0xFF);
	close(afd);
}

void battery(void)
{
	int result;
	size_t len = sizeof(result);

	if(sysctlbyname("hw.acpi.battery.life", &result, &len, NULL, 0) == -1) return;
	if(result < 0) return;

	snprintf(buf, MAX_BUF_SIZE, "%s BAT %3d%%", buf, result);
}

void keymap(void)
{
	XkbStateRec state;

	if(XkbGetState(dpy, XkbUseCoreKbd, &state)) return;
	XkbDescPtr desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);

	//snprintf(result, 3, "%s", KEYMAP_GROUP);
  snprintf(buf, MAX_BUF_SIZE, "%s [%.2s]", buf, KEYMAP_GROUP); 
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
		keymap();
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
