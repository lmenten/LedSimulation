
#include <Adafruit_GFX.h>    // Core graphics library
#include <Sam_TFTLCD.h> // Hardware-specific library

class LedSimulation {
  private:
    Adafruit_TFTLCD *pTft;
    unsigned char Scene[16][16];
    unsigned char SceneWas[16][16];
  public:
    LedSimulation(Adafruit_TFTLCD &tft);
    void clear(void);
	void clearNow(void);
    void clearScene(void);
    void display(void);
    void draw(void);
    void setScene(unsigned char Scene[16][16]);
    void setLedOn(int x, int y);
	void setLedRed(int x, int y);
    void setLedBlue(int x, int y);
	bool getLedState(int x, int y);
    void setLedOff(int x, int y);
    void setLedOnNow(int x, int y);
    void setLedOffNow(int x, int y);
};
