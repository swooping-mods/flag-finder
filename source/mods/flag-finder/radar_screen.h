#ifndef FLAG_FINDER_RADAR_SCREEN_H
#define FLAG_FINDER_RADAR_SCREEN_H
/* The preview and Unity upload use this same bounded RGBA display renderer.
 * Coordinates use a top-left origin; upload reverses rows for Unity's UVs. */
#define RADAR_WIDTH 320
#define RADAR_HEIGHT 256
typedef struct {unsigned char r,g,b,a;} RadarPixel;
typedef struct {int filter,found;float metres,bearing_x,bearing_y;unsigned phase;} RadarReading;
static void radar_pixel(RadarPixel *p,int x,int y,RadarPixel c){if(x>=0&&x<RADAR_WIDTH&&y>=0&&y<RADAR_HEIGHT)p[y*RADAR_WIDTH+x]=c;}
static void radar_line(RadarPixel *p,int x,int y,int u,int v,RadarPixel c){
 int dx=u>x?u-x:x-u,dy=v>y?y-v:v-y,sx=x<u?1:-1,sy=y<v?1:-1,e=dx+dy;
 for(;;){radar_pixel(p,x,y,c);if(x==u&&y==v)break;int q=2*e;if(q>=dy){e+=dy;x+=sx;}if(q<=dx){e+=dx;y+=sy;}}
}
static const unsigned char *radar_glyph(char c){
 /* Five columns, seven rows, generated from deliberately simple instrument lettering. */
 static const unsigned char glyphs[][5]={
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},{24,20,18,127,16},
 {39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30},
 {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},{127,73,73,73,65},
 {127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},
 {127,8,20,34,65},{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},
 {127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},
 {63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67},
 {0,0,0,0,0},{0,8,8,8,0}};
 if(c>='0'&&c<='9')return glyphs[c-'0'];
 if(c>='A'&&c<='Z')return glyphs[10+c-'A'];
 return glyphs[c=='-'?37:36];
}
static void radar_text(RadarPixel *p,int x,int y,const char *s,int scale,RadarPixel c){
 for(;*s;s++,x+=6*scale){const unsigned char *g=radar_glyph(*s);for(int a=0;a<5;a++)for(int b=0;b<7;b++)if(g[a]&(1<<b))for(int i=0;i<scale;i++)for(int j=0;j<scale;j++)radar_pixel(p,x+a*scale+i,y+b*scale+j,c);}
}
static void radar_draw(RadarPixel *p,RadarReading r){
 static const float directions[16][2]={{0,-1},{.382683f,-.923880f},{.707107f,-.707107f},{.923880f,-.382683f},{1,0},{.923880f,.382683f},{.707107f,.707107f},{.382683f,.923880f},{0,1},{-.382683f,.923880f},{-.707107f,.707107f},{-.923880f,.382683f},{-1,0},{-.923880f,-.382683f},{-.707107f,-.707107f},{-.382683f,-.923880f}};
 RadarPixel ink={114,239,214,255},muted={39,112,111,255},amber={255,160,57,255};
 unsigned phase=r.phase%16;
 for(int y=0;y<RADAR_HEIGHT;y++)for(int x=0;x<RADAR_WIDTH;x++){
  int dx=x-160,dy=y-133,d=dx*dx+dy*dy;RadarPixel c={7,20,25,255};
  if(d<=78*78){c=(RadarPixel){9,28,32,255};float dot=dx*directions[phase][0]+dy*directions[phase][1];if(dot>0&&dot*dot>.94f*d)c=(RadarPixel){13,49,48,255};
   if((d>25*25-50&&d<25*25+50)||(d>51*51-102&&d<51*51+102)||(d>78*78-234))c=muted;
   if(dx==0||dy==0)c=(RadarPixel){22,65,67,255};
  }
  p[y*RADAR_WIDTH+x]=c;
 }
 radar_text(p,34,20,"FLAG FINDER",2,ink);
 radar_line(p,28,43,292,43,muted);
 radar_text(p,31,53,r.filter==0?"FLAGS":r.filter==1?"SITES":"ALL",1,amber);radar_text(p,267,53,"80M",1,muted);
 radar_line(p,155,50,160,46,ink);radar_line(p,160,46,165,50,ink);
 radar_line(p,160,129,160,137,ink);radar_line(p,156,133,164,133,ink);
 if(r.found&&r.metres>=0&&r.metres<=80){
  float x=r.bearing_x,y=r.bearing_y,n=x*x+y*y;
  /* Callers supply a normalized planar bearing. Reject corrupt projections. */
  if(n<=1.01f){int u=160+(int)(x*r.metres*.95f),v=133-(int)(y*r.metres*.95f);
   for(int j=-3;j<=3;j++)for(int i=-3;i<=3;i++)if(i*i+j*j<=10)radar_pixel(p,u+i,v+j,amber);
   radar_line(p,u-6,v-6,u-3,v-6,ink);radar_line(p,u-6,v-6,u-6,v-3,ink);radar_line(p,u+6,v+6,u+3,v+6,ink);radar_line(p,u+6,v+6,u+6,v+3,ink);
  }
 }
 radar_line(p,28,220,292,220,muted);
 char range[7]="-- M";if(r.found&&r.metres>=0&&r.metres<=80){int n=(int)(r.metres+.5f);range[0]=(char)('0'+n/10);range[1]=(char)('0'+n%10);if(n<10)range[0]=' ';}
 radar_text(p,31,231,range,1,ink);radar_text(p,227,231,"FILTER",1,muted);
 if(!r.found)radar_text(p,133,231,"NO SIGNAL",1,muted);
}
#endif
