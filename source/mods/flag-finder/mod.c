#define MOD_INPUT_ACTIONS {{"flag-finder.cycle","T",'T'}}
#define FEATURE 8
#define MOD_ID "flag-finder"
#define HUD_Y 280
#define FW_NAME "Flag Finder"
#define FW_DESC "Handheld radar for nearby unraised flags and findable sites."
#define FW_PRICE 125
#define FW_TEMPLATE 0
#include "runtime.h"
#include "../../runtime/sdk/scene/static_model_pbr.h"
#include "assets/generated/model_data.h"
#include "radar_screen.h"
static int nearest=-1;static float metres=0;
static DRoot radar,radar_camera;static SMResources radar_resources;static SURoot radar_pixels;
static PTR radar_set_pixels,radar_apply;static float radar_next_frame,radar_retry;
static RadarPixel radar_frame[RADAR_WIDTH*RADAR_HEIGHT],radar_upload[RADAR_WIDTH*RADAR_HEIGHT];
static void radar_remove(void){
 destroy_root(&radar);ddrop(&radar_camera);su_drop(&radar_pixels);sm_release(&radar_resources);radar_next_frame=radar_retry=0;
}
static int radar_prepare(void){
 if(!su.ready&&!su_bind(GetModuleHandleA("GameAssembly.dll")))return 0;
 radar_set_pixels=su_method("UnityEngine","Texture2D","SetPixels32","UnityEngine.Color32[]");
 radar_apply=su_method("UnityEngine","Texture2D","Apply","System.Boolean;System.Boolean");
 PTR colour=find_class("UnityEngine","Color32");
 if(!radar_set_pixels||!radar_apply||!colour||!sm_prepare_pbr(&rd_model,&radar_resources,rd_pbr,2))return 0;
 radar_pixels=su_hold(su_array(colour,RADAR_WIDTH*RADAR_HEIGHT,0,4));if(!radar_pixels)return 0;
 int ok=1;PTR material=su_get(radar_resources.materials[1]);
 /* Lit fallbacks retain an unlit screen through texture-only emission. */
 SUColor black={0,0,0,1};void *base[]={&black};call(su.material_color,material,base,&ok);
 if(!sm_colour(material,"_BaseColor",black)||!sm_colour(material,"_EmissionColor",(SUColor){1,1,1,1}))return 0;
 void *texture[]={su.string("_EmissionMap"),su_get(radar_resources.textures[3])};
 call(su_method("UnityEngine","Material","SetTexture","System.String;UnityEngine.Texture"),material,texture,&ok);
 void *keyword[]={su.string("_EMISSION")};call(su.material_keyword,material,keyword,&ok);
 /* Preserve readable lettering at the small camera-held size. */
 int filter=1;void *filter_args[]={&filter};call(su_method("UnityEngine","Texture","set_filterMode","UnityEngine.FilterMode"),su_get(radar_resources.textures[3]),filter_args,&ok);
 return ok;
}
/* Read the active view without claiming or mutating shared camera resources.
 * Project onto the moon tangent plane; straight-up/down views use screen up. */
static int radar_bearing(DVec player,DVec target,DVec forward,DVec camera_up,float *x,float *y){
 *x=*y=0;if(!(vdot(player,player)>.0001f))return 0;
 DVec up=normal(player),delta=vadd(target,vmul(up,-vdot(target,up)));
 float length=vdot(delta,delta);if(!(length>=0))return 0;
 if(length<=.000001f)return 1;
 forward=vadd(forward,vmul(up,-vdot(forward,up)));
 if(vdot(forward,forward)<.0001f)forward=vadd(camera_up,vmul(up,-vdot(camera_up,up)));
 if(!(vdot(forward,forward)>.0001f))return 0;
 forward=normal(forward);delta=normal(delta);
 DVec right={up.y*forward.z-up.z*forward.y,up.z*forward.x-up.x*forward.z,up.x*forward.y-up.y*forward.x};
 *x=vdot(delta,right);*y=vdot(delta,forward);return 1;
}
static void radar_hold(PTR walker,int visible){
 if(!visible){radar_remove();return;}if(m.time<radar_retry)return;
 int ok=1;PTR camera=0;PTR field=get_field(object_class(walker),"viewCamera");if(field)field_value(walker,field,&camera);
 if(!malive(camera)){radar_remove();return;}
 /* A surviving old camera must not retain the new local viewmodel. */
 if(radar_camera&&dget(radar_camera)!=camera)radar_remove();
 PTR camera_transform=call(m.transform,camera,0,&ok);if(!ok||!camera_transform){radar_remove();return;}
 if(!malive(dget(radar))){
  radar_remove();radar_camera=dhold(camera);if(!radar_camera||!radar_prepare())goto fail;
  PTR root=sm_attach(&rd_model,&radar_resources,camera_transform,2,"MoonLab held Flag Finder");
  if(!root)goto fail;radar=dhold(root);if(!radar){su_destroy(root);goto fail;}
  PTR tr=su_transform(root);DVec position={.25f,-.17f,.52f};void *args[]={&position};call(su.tr_local_position,tr,args,&ok);
  /* These rigid parts have no colliders; omit viewmodel shadows. */
  PTR child=su_method("UnityEngine","Transform","GetChild","System.Int32");
  for(int i=0;i<2;i++){void *index[]={&i};PTR ct=call(child,tr,index,&ok),go_id=call(su_method("UnityEngine","Component","get_gameObject",""),ct,0,&ok);
   PTR renderer=su_component(go_id,"MeshRenderer");int off=0;unsigned char no=0;void *cast[]={&off},*receive[]={&no};
   call(su_method("UnityEngine","Renderer","set_shadowCastingMode","UnityEngine.Rendering.ShadowCastingMode"),renderer,cast,&ok);
   call(su_method("UnityEngine","Renderer","set_receiveShadows","System.Boolean"),renderer,receive,&ok);
  }
  if(!ok)goto fail;mevent("radar-model-created");
 }
 if(m.time<radar_next_frame)return;radar_next_frame=m.time+.1f;
 RadarReading reading={(int)(m.known[0]%3),nearest>=0,metres,0,0,(unsigned)(m.time*4)};
 if(reading.found){
  DVec player=mpos(walker,&ok),target=direction(dget(m.world),nearest,&ok);
  DVec forward=dvector(su.tr_forward,camera_transform,&ok),camera_up=dvector(su.tr_up,camera_transform,&ok);
  if(!radar_bearing(player,target,forward,camera_up,&reading.bearing_x,&reading.bearing_y))reading.found=0;
  if(!ok)reading.found=0;
 }
 radar_draw(radar_frame,reading);
 for(int y=0;y<RADAR_HEIGHT;y++)su_copy(radar_upload+(RADAR_HEIGHT-1-y)*RADAR_WIDTH,radar_frame+y*RADAR_WIDTH,RADAR_WIDTH*4);
 PTR array=su_get(radar_pixels),bytes=su.array_address(array,4,0);if(!bytes)goto fail;
 su_copy(bytes,radar_upload,sizeof(radar_upload));void *pixels[]={array};
 call(radar_set_pixels,su_get(radar_resources.textures[3]),pixels,&ok);
 unsigned char yes=1,no=0;void *apply[]={&yes,&no};call(radar_apply,su_get(radar_resources.textures[3]),apply,&ok);
 if(ok)return;
fail:
 radar_remove();radar_retry=m.time+5;mevent("radar-model-unavailable");
}
static void feature_end(void){fw_clear();radar_remove();nearest=-1;metres=0;m.active_tool=0;m.next_scan=0;}
static void feature_tick(PTR walker,PTR world,int input){
 int ok=1,blocked=boolean(m.menu,walker,&ok)||boolean(m.blocked,walker,&ok);m.active_tool=fw_equipped(walker);
 if(!input||!ok||blocked||!m.owned||!m.active_tool){feature_end();return;}
 if(m.owned&&m.active_tool&&m.pressed['T']&&!blocked){m.known[0]=(unsigned char)((m.known[0]+1)%3);m.dirty=1;save();m.next_scan=0;}
 if(m.owned&&m.active_tool&&!blocked&&m.time>=m.next_scan){
  m.next_scan=m.time+.2f;nearest=-1;float r=radius(world);DVec player=mpos(walker,&ok);float best=6400;
  MoonMapWorld source={&m.mapping,world};MoonMapQuery query=moon_map_query(&source);
  if(ok)nearest=moon_map_nearest(&query,(MoonMapPoint){player.x,player.y,player.z},r,m.known[0],6400,&best);
  metres=msqrt(best);
 }
 PTR screen=moon_map_screen(&m.mapping,walker,&ok);
 int opened=screen?boolean(m.mapping.map_open,screen,&ok):1;
 int first=boolean(m.first,walker,&ok);
 radar_hold(walker,ok&&!opened&&first);
 char key[64]="T",text[220];moon_input_label("flag-finder.cycle","T",key,sizeof(key));char *p=cat(cat(text,key),": Flags / sites / all | ");
 if(nearest<0)cat(p,"No matching findable within 80 m");else{p=cat(p,"Findable ");p=num(p,(U64)(metres+.5f));cat(p," m");}
 fw_hints->publish(MOD_ID,walker,text);
}
