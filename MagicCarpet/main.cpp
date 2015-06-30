#include <irrlicht.h>
#include "MastEventReceiver.cpp"
#include "Arena.h"
#include "Player.h"
#include "EmptyControl.h"
#include "KeyboardControl.h"
#include "AIControl.h"
#include "MouseControl.h"
#include "WiiControl.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif



bool isHitting(Player* hitter, Player* victim)
{
	aabbox3d<f32> boxHitter;
	aabbox3d<f32> boxVictim;
	boxHitter = hitter->getStick()->getNode()->getTransformedBoundingBox();
	boxVictim = victim->getCharacter()->getNodeUpper()->getTransformedBoundingBox();
	hitter->getNode()->getRelativeTransformation().transformBoxEx(boxHitter);
	victim->getNode()->getRelativeTransformation().transformBoxEx(boxVictim);
	return boxHitter.intersectsWithBox(boxVictim);
}

bool collision(ISceneNode* node1, ISceneNode* node2)
{
	aabbox3d<f32> box1 = node1->getTransformedBoundingBox();
	aabbox3d<f32> box2 = node2->getTransformedBoundingBox();
	return box1.intersectsWithBox(box2);
}

int main()
{
    // initial settings
    const wchar_t* windowCaption = L"Magic Carpet";
    dimension2d<u32> windowSize(800,600);
    u32 bitDepth = 32;
    bool fullscreen = false;
    bool shadows = false;
	bool vSync = false;

    // event receiver
    MastEventReceiver eventReceiver;
    eventReceiver.init();

    // irrlicht device
    IrrlichtDevice* device = createDevice(
                                 EDT_OPENGL,
                                 //EDT_BURNINGSVIDEO,
                                 //EDT_SOFTWARE,
                                 windowSize, bitDepth,
                                 fullscreen, shadows, vSync, &eventReceiver);
    if (!device) return 1;
    device->setWindowCaption(windowCaption);
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
	ISceneCollisionManager* cmgr = smgr->getSceneCollisionManager();
	ILogger* logger = device->getLogger();

    Arena* arena = new Arena(driver, smgr);

    Player* player1 = new Player(driver, smgr);
    player1->getNode()->setPosition(vector3df(250,25,221));
    player1->getNode()->setRotation(vector3df(0,0,0));
    player1->setControl(new KeyboardControl(player1, &eventReceiver));
    //player->setControl(new MouseControl(player, &eventReceiver, windowSize.Width/2, windowSize.Height/2));

    Player* player2 = new Player(driver, smgr);
    player2->getNode()->setPosition(vector3df(-213,25,-123));
    player2->getNode()->setRotation(vector3df(0,180,0));
	player2->setControl(new EmptyControl(player2));
	//player2->setControl(new AIControl(player2));

	// target is at 0,25,0
	IBillboardSceneNode* bill = smgr->addBillboardSceneNode(0, dimension2d<f32>(20,20), vector3df(0,25,0));
	bill->setMaterialTexture(0, driver->getTexture("data/target.png"));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    // cameras
    ICameraSceneNode* camFirst = smgr->addCameraSceneNode(player1->getNode(), vector3df(-0.2f, 8.5f, -1.4f));
    ICameraSceneNode* camThird = smgr->addCameraSceneNode(player1->getNode(), vector3df(0, 10, 15));
    ICameraSceneNode* camBird = smgr->addCameraSceneNode(player1->getNode(), vector3df(0, 100, 35));
    ICameraSceneNode* camStatic = smgr->addCameraSceneNode(0, vector3df(0, 150, 0));
    smgr->setActiveCamera(camThird);

    // camera targets
    vector3df camFirstTarget;
    vector3df camThirdTarget;
    vector3df camBirdTarget;
    vector3df camStaticTarget;

	// viewports
	rect<s32> viewportFull = rect<s32>(0, 0, windowSize.Width, windowSize.Height);
	rect<s32> viewport1 = rect<s32>(0, 0, windowSize.Width/2, windowSize.Height/2);
	rect<s32> viewport2 = rect<s32>(windowSize.Width/2, 0, windowSize.Width, windowSize.Height/2);
	rect<s32> viewport3 = rect<s32>(0, windowSize.Height/2, windowSize.Width/2, windowSize.Height);
	rect<s32> viewport4 = rect<s32>(windowSize.Width/2, windowSize.Height/2, windowSize.Width, windowSize.Height);

    // frame rate display
    s32 lastFPS = -1;

    const int TICKS_PER_SECOND = 50;
    const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
    const int MAX_FRAMESKIP = 10;
    u32 next_game_tick = device->getTimer()->getTime();
    int loops;

    // main loop
    while (device->run())
    {

        loops = 0;
        while (device->getTimer()->getTime() > next_game_tick && loops < MAX_FRAMESKIP)
        {

            // stop receiving events
            eventReceiver.endEventProcess();

			// switch views
            if (eventReceiver.keyPressed(KEY_KEY_V))
            {
                if (smgr->getActiveCamera() == camFirst)
                    smgr->setActiveCamera(camThird);
                else if (smgr->getActiveCamera() == camThird)
                    smgr->setActiveCamera(camBird);
                else if (smgr->getActiveCamera() == camBird)
                    smgr->setActiveCamera(camStatic);
                else if (smgr->getActiveCamera() == camStatic)
                    smgr->setActiveCamera(camFirst);
            }
            // switch arena appearances
            if (eventReceiver.keyPressed(KEY_KEY_M))
            {
                arena->setAppearance((arena->getAppearance()+1)%2);
            }

            // update players
			player1->update(TICKS_PER_SECOND);
			player2->update(TICKS_PER_SECOND);

			// DEBUG
			player1->getStick()->getNode()->setDebugDataVisible(EDS_BBOX);
			player2->getStick()->getNode()->setDebugDataVisible(EDS_BBOX);
			player1->getCharacter()->getNodeUpper()->setDebugDataVisible(EDS_BBOX);
			player2->getCharacter()->getNodeUpper()->setDebugDataVisible(EDS_BBOX);			
			
			// TODO: collision detection, border detection			
			aabbox3d<f32> boxStick1 = player1->getStick()->getNode()->getTransformedBoundingBox();
			aabbox3d<f32> boxStick2 = player2->getStick()->getNode()->getTransformedBoundingBox();
			aabbox3d<f32> boxCharacter1 = player1->getCharacter()->getNodeUpper()->getTransformedBoundingBox();
			aabbox3d<f32> boxCharacter2 = player2->getCharacter()->getNodeUpper()->getTransformedBoundingBox();
			// TODO: problem with bounding boxes of character
			if(boxStick1.intersectsWithBox(boxStick2))
			{
				logger->log("Player2 got hit!");
			}
			if(boxStick2.intersectsWithBox(boxStick1))
			{
				logger->log("Player1 got hit!");
			}
			
			// temporary
			vector3df pos = player1->getNode()->getPosition();
			vector3df rot = player1->getNode()->getRotation();
    		f32 rotCos = cos(rot.Y / 180 * PI);
    		f32 rotSin = sin(rot.Y / 180 * PI);
            // update camera targets
            // TODO: in player class, update only active camera
            camFirstTarget.X = pos.X - rotSin * 10;
            camFirstTarget.Z = pos.Z - rotCos * 10;
            camFirstTarget.Y = pos.Y + 4;
            camFirst->setTarget(camFirstTarget);
            camThirdTarget.X = pos.X - rotSin * 10;
            camThirdTarget.Z = pos.Z - rotCos * 10;
            camThirdTarget.Y = pos.Y + 0;
            camThird->setTarget(camThirdTarget);
            camBirdTarget.X = pos.X - rotSin * 60;
            camBirdTarget.Z = pos.Z - rotCos * 60;
            camBirdTarget.Y = pos.Y + 0;
            camBird->setTarget(camBirdTarget);
            camStaticTarget = pos;
            camStatic->setTarget(camStaticTarget);

            next_game_tick += SKIP_TICKS;
            loops++;

            // start receiving events
            eventReceiver.startEventProcess();
        }

        // rendering
        driver->beginScene(true, true, SColor(255, 0, 0, 0));
        smgr->drawAll();
        driver->endScene();

		/*
        // rendering (multiple viewports)
        driver->beginScene(true, true, SColor(255, 0, 0, 0));
        driver->setViewPort(viewport1);
        smgr->setActiveCamera(camFirst);
        smgr->drawAll();
        driver->setViewPort(viewport2);
        smgr->setActiveCamera(camThird);
        smgr->drawAll();
        driver->setViewPort(viewport3);
        smgr->setActiveCamera(camBird);
        smgr->drawAll();
        driver->setViewPort(viewport4);
        smgr->setActiveCamera(camStatic);
        smgr->drawAll();
        driver->endScene();
		*/

        // display FPS
        s32 fps = driver->getFPS();
        if (lastFPS != fps)
        {
            stringw tmp(windowCaption);
            tmp += L" [";
            tmp += driver->getName();
            tmp += L"] [fps: ";
            tmp += fps;
            tmp += L"]";
            device->setWindowCaption(tmp.c_str());
            lastFPS = fps;
        }
    }

    // clean up
    device->drop();
    return 0;
}
