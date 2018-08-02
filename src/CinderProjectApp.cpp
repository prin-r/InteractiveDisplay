#include "cinder/app/AppNative.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"
#include "cinder/app/AppImplMsw.h"
#include "boost/thread/mutex.hpp"
#include "CinderOpenCV.h"

#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

#include "cinder/CinderResources.h"

#if defined(WIN32) && !defined(_DEBUG)
#include "AppVerify.h"
#endif

#include "Config.h"
#include "Debug.h"
#include "BitException.h"
#include "BitExceptionHandler.h"
#include "BitShortcutKey.h"

#include "AClass.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define RES_VERT_GLSL		CINDER_RESOURCE( ../resources/, mandelbrot_vert.glsl, 150, GLSL )
#define RES_FRAG_GLSL		CINDER_RESOURCE( ../resources/, mandelbrot_frag.glsl, 151, GLSL )

class MahanakhonParkingTestApp : public AppNative {
public:
	void prepareSettings(Settings *settings);
	void setup();
	void shutdown();
	void mouseDown(MouseEvent event);
	void keyDown(KeyEvent event);
	void update();
	void draw();
	void emitClose();

	void rotationPattern(int k, float maxAngle);
	void normalScalePattern(float k, float direction);
	void decreaseFromLeft(int k, float lowerBound);
	void scaleWithOrder(int k);
	void general(float scale, float rotation);

	void drawCharPattern1(float timeT);

	double angle = 0.0;
	clock_t begin_time = clock();
	float progressTime = 0.0;
	int presentState = 0;
	float presentScale = 1.0f;
	float timeLenght = 5.0f;

	int units_h = 1040;
	int units_w = 40;
	int xOffset = 10;
	int yOffset = 10;
	int rectSize = 20;

	gl::Texture texture;

	gl::GlslProg mGlsl;

	std::vector<vector<float>> scaleArray;

private:
#if defined(WIN32) && !defined(_DEBUG)
	AppVerify  appVerify_;
#endif
	void toggleFullscreen();
	void toggleDisplayParameters();
	void checkExceptionFromThread();

	Bit::Config config_;
	bool      borderless_;

	// debugging values
	bool      debug_;
	bool      paramsOn_;
	Bit::ShortcutKey shortcutKey_;
	bool setupCompleted_;
	AClass aClass_;

};

void MahanakhonParkingTestApp::prepareSettings(Settings *settings)
{
#if defined(WIN32) && !defined(_DEBUG)
	//if( !appVerify_.isValid() ) {
	//	showCursor();
	//	MessageBox( NULL, L"Application has expired.", L"ERROR", MB_OK );	
	//	exit( 1 );
	//}
#endif

	// initialize debugging values
	debug_ = false;
	paramsOn_ = false;
	setupCompleted_ = false;

	try {
		// register shutdown function to exception handler
		Bit::ExceptionHandler::registerShutdownFunction(std::bind(&MahanakhonParkingTestApp::emitClose, this));
		// read settings for window setup
		config_.readConfig();
		Bit::Config::DisplayConfig appConfig = config_.getDisplayConfig();
		// setup window 
		settings->setWindowSize(appConfig.windowSize.x, appConfig.windowSize.y);
		settings->setWindowPos(appConfig.windowPos.x, appConfig.windowPos.y);
		settings->setAlwaysOnTop(appConfig.alwaysOnTop);
		settings->setBorderless(appConfig.borderless);
		borderless_ = appConfig.borderless;
		// setup cursor
		if (appConfig.hideCursor)
			hideCursor();
		else
			showCursor();
	}
	catch (std::exception& e) {
		Bit::ExceptionHandler::handleException(&e);
	}
}

void MahanakhonParkingTestApp::setup()
{

	scaleArray.resize(1040);
	for (unsigned i = 0; i < scaleArray.size(); i++) {
		scaleArray[i].resize(400);
		for (unsigned j = 0; j < scaleArray[i].size(); j++) {
			scaleArray[i][j] = 1.0f;
		}
	}

	texture = loadImage(loadAsset("2_char_h.png"));
	texture.enableAndBind();

	mGlsl = gl::GlslProg(loadAsset("mandelbrot_vert.glsl"), loadAsset("mandelbrot_frag.glsl"));

	// setup shortcut keys
	shortcutKey_.setupDisplayDialog("Shortcut list", Vec2i(400, 200), ColorA(0.3f, 0.3f, 0.3f, 0.4f));
	shortcutKey_.addShortcut(KeyEvent::KEY_ESCAPE, std::bind(&MahanakhonParkingTestApp::emitClose, this), "close application");
	shortcutKey_.addShortcut('d', &debug_, "toggle display debug mode");
	shortcutKey_.addShortcut('f', std::bind(&MahanakhonParkingTestApp::toggleFullscreen, this), "toggle fullscreen mode");
	shortcutKey_.addShortcut('p', std::bind(&MahanakhonParkingTestApp::toggleDisplayParameters, this), "toggle display parameters dialog");
	shortcutKey_.addShortcut(KeyEvent::KEY_F1, std::bind(&Bit::ShortcutKey::toggleDisplay, &shortcutKey_), "toggle display shortcut keys list");

	try {
		config_.setup();

		// setup everything here
		config_.readConfigurableConfig(aClass_, "aClassConfig");	// this will eventually calls AClass::readConfig() with the Bit::JsonTree* node named "aClassConfig" as argument
		config_.readConfigurableParams(aClass_, "aClassParams");	// this will eventually calls AClass::readParams() with the Bit::JsonTree* node named "aClassParams" as argument

		// mark setup complete at the end of setup.
		setupCompleted_ = true;
	}
	catch (std::exception& e) {
		Bit::ExceptionHandler::handleException(&e);

	}
}

void MahanakhonParkingTestApp::emitClose()

{
	// if setup is donw (we have window), emit the same event like clicking windows' close button (X button) on upper right corner
	// TODO: we need to handle multiple windows later
	if (setupCompleted_){
		WindowImplMsw* impl = reinterpret_cast<WindowImplMsw*>(::GetWindowLongPtr((HWND)ci::app::getWindow()->getNative(), GWLP_USERDATA));
		impl->getWindow()->emitClose();
		impl->privateClose();
		delete impl;
		// quit will call shutdown() for clean up and close the app
		quit();
	}
	else{	// otherwise, simply exit
		exit(Bit::Exception::getExitCode());
	}
}

void MahanakhonParkingTestApp::shutdown()
{
	//int exitCode = Bit::Exception::getExitCode();
	//exit( exitCode );	// we can not exit() here as memory leaks will occur
}

void MahanakhonParkingTestApp::toggleFullscreen()
{
	setFullScreen(!isFullScreen());
}

void MahanakhonParkingTestApp::toggleDisplayParameters()
{
	paramsOn_ = !paramsOn_;
	if (paramsOn_) {
		showCursor();

		config_.showParams();
	}
	else {
		hideCursor();

		config_.hideParams();
	}
}

void MahanakhonParkingTestApp::keyDown(KeyEvent event)
{
	shortcutKey_.keyDown(event);
}

void MahanakhonParkingTestApp::mouseDown(MouseEvent event)
{
}

void MahanakhonParkingTestApp::update()
{

	progressTime = float((clock() - begin_time)*0.001 / timeLenght);

	if (presentState == 0) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentScale = 0.0f;
			presentState += 1;
		}
		else {
			presentScale = progressTime + 0.0f;
		}
	}
	else if (presentState == 1) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentScale = 0.0f;
			presentState += 1;
		}
		else {
			presentScale = 1.0f - progressTime + 0.0f;
		}
	}
	else if (presentState == 2) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentScale = 0.0f;
			presentState += 1;
		}
		else {
			presentScale = progressTime + 0.0f;
		}
	}
	else if (presentState == 3) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = progressTime + 0.0f;
		}
	}
	else if (presentState == 4) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = 1.0f - progressTime;
		}
	}
	else if (presentState == 5) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = 1.0f - progressTime;
		}
	}
	else if (presentState == 6) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = progressTime;
		}
	}
	else if (presentState == 7) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {

		}
	}
	else if (presentState == 8) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = progressTime;
		}
	}
	else if (presentState == 9) {
		if (progressTime > 1.0f) {
			begin_time = clock();
			presentState += 1;
		}
		else {
			presentScale = 1 - progressTime;
		}
	}
	else if (presentState == 10) {
		if (progressTime > 0.2f) {
			begin_time = clock();
			presentState = 0;
		}
		else {
			angle = 0.0f;
		}
	}

	//cv::Mat tmp = toOcv(texture).mul(toOcv(texture_trans) + progressTime);

	try {
		// check if there is any exception from thread, for more info see Bit::ExceptionHandler::checkExceptionFromThread
		Bit::ExceptionHandler::checkExceptionFromThread();

		// added update part here


	}
	catch (std::exception& e) {
		Bit::ExceptionHandler::handleException(&e);
	}
}

float clamp(float a, float min, float max) {
	if (a > max) return max;
	if (a < min) return min;
	return a;
}

void MahanakhonParkingTestApp::rotationPattern(int k, float maxAngle) {
	angle = maxAngle*progressTime + 0.0;
	for (int j = 0; j < units_h; j++) {
		for (int i = 0; i < units_w; i++) {
			ci::gl::pushMatrices();
			int x1 = xOffset + i*rectSize*0.5f;
			int y1 = yOffset + j*rectSize*0.5f;
			float scale = rectSize * presentScale * 0.25;
			ci::gl::translate(x1 - scale / 2.0, y1 - rectSize / 2.0);

			if (j < k) {
				float angleScaleByDistance = clamp(sqrt(j + 2.0f) * angle, -maxAngle, maxAngle);
				ci::gl::rotate(-angleScaleByDistance);
				ci::gl::scale(scale, 1);
			}
			else {
				float angleScaleByDistance = clamp(sqrt(abs(units_h - j) + 1.0f) * angle, -maxAngle, maxAngle);
				ci::gl::rotate(angleScaleByDistance);
				ci::gl::scale(scale, 1);
			}
			ci::gl::drawSolidRect(ci::Rectf(-rectSize / 8.0, -rectSize / 8.0, rectSize / 8.0, rectSize / 8.0));
			ci::gl::popMatrices();
		}
	}
}

void MahanakhonParkingTestApp::normalScalePattern(float k, float direction) {
	for (int j = 0; j < units_h; j++) {
		for (int i = 0; i < units_w; i++) {
			ci::gl::pushMatrices();
			int x1 = xOffset + i*rectSize*0.5f;
			int y1 = yOffset + j*rectSize*0.5f;
			float order = 1.0f;
			if (direction >= -0.01f) {
				order = (abs(direction - i));
			}
			float scale = k * rectSize * clamp(presentScale * order, 0.0f, 1.0f);
			ci::gl::translate(x1 - scale / 2.0, y1 - rectSize / 2.0);
			ci::gl::rotate(angle);
			ci::gl::scale(scale, 1);
			ci::gl::drawSolidRect(ci::Rectf(-rectSize / 8.0, -rectSize / 8.0, rectSize / 8.0, rectSize / 8.0));
			ci::gl::popMatrices();
		}
	}
}

void MahanakhonParkingTestApp::decreaseFromLeft(int k, float lowerBound) {
	float scaleLim = presentScale*rectSize*units_w;
	for (int j = 0; j < units_h; j++) {
		float sumScale = 0.0;
		float scaleLimRow = presentScale*scaleLim + (1 - presentScale)*(int(sqrt(abs(j - k) + 1)))*scaleLim;
		for (int i = units_w - 1; i > 0; i--) {
			ci::gl::pushMatrices();
			int x1 = xOffset + i*rectSize*0.5f;
			int y1 = yOffset + j*rectSize*0.5f;
			float scale = rectSize;
			sumScale += scale;
			ci::gl::translate(x1 - scale / 2.0, y1 - rectSize / 2.0);
			ci::gl::rotate(angle);
			if (sumScale - scale > scaleLimRow) {
				ci::gl::scale(lowerBound, 1);
			}
			else if (sumScale - scale <= scaleLimRow){
				if (sumScale >= scaleLimRow) {
					scale = abs(scaleLimRow - (sumScale - scale));
				}
				ci::gl::scale(clamp(scale, lowerBound, 1000.0f), 1);
			}
			ci::gl::drawSolidRect(ci::Rectf(-rectSize / 8.0, -rectSize / 8.0, rectSize / 8.0, rectSize / 8.0));
			ci::gl::popMatrices();
		}
	}
}

void MahanakhonParkingTestApp::general(float scale, float rotation) {
	for (int j = 0; j < units_h; j++) {
		for (int i = 0; i < units_w; i++) {
			ci::gl::pushMatrices();
			int x1 = xOffset + i*rectSize*0.5f;
			int y1 = yOffset + j*rectSize*0.5f;
			float _scale = rectSize * scale;
			ci::gl::translate(x1 - _scale / 2.0, y1 - rectSize / 2.0);
			ci::gl::rotate(rotation);
			ci::gl::scale(_scale, 1);
			ci::gl::drawSolidRect(ci::Rectf(-rectSize / 8.0, -rectSize / 8.0, rectSize / 8.0, rectSize / 8.0));
			ci::gl::popMatrices();
		}
	}
}

void MahanakhonParkingTestApp::scaleWithOrder(int k) {
	for (int j = 0; j < units_h; j++) {
		for (int i = 0; i < units_w; i++) {
			ci::gl::pushMatrices();
			int x1 = xOffset + i*rectSize*0.5f;
			int y1 = yOffset + j*rectSize*0.5f;
			float scale = rectSize * (1 - presentScale);

			float scaleOrder = clamp(presentScale * presentScale * ((i*j + 26.0 + 0.0) / (sqrt(abs(k - j*j)+26.0) + 1.0)) * rectSize, 0.0, rectSize);

			ci::gl::translate(x1 - scaleOrder / 2.0, y1 - rectSize / 2.0);
			ci::gl::rotate(angle);
			ci::gl::scale(scaleOrder, 1);
			ci::gl::drawSolidRect(ci::Rectf(-rectSize / 8.0, -rectSize / 8.0, rectSize / 8.0, rectSize / 8.0));
			ci::gl::popMatrices();
		}
	}
}

void MahanakhonParkingTestApp::drawCharPattern1(float timeT) {
	gl::enableAlphaBlending();
	mGlsl.bind();

	mGlsl.uniform("tex", 0);
	mGlsl.uniform("isWhite", false);
	mGlsl.uniform("scale", timeT);

	gl::draw(texture, Rectf(-25, 30-15, 374, 1064-15));

	mGlsl.unbind();
	gl::disableAlphaBlending();
}


void MahanakhonParkingTestApp::draw()
{
	if (!setupCompleted_)
		return;

	// clear out the window with black

	gl::clear(Color(0, 0, 0));

	// draw everything here
	if (presentState == 0) {
		normalScalePattern(1.0f, -1.0f);
	}
	else if (presentState == 1) {
		decreaseFromLeft(0, 0.5f);
	}
	else if (presentState == 2) {
		timeLenght = 10.0f;
		scaleWithOrder(0);
	}
	else if (presentState == 3) {
		//timeLenght = 5.0f;
		general(1.0f, angle);
		drawCharPattern1(presentScale);
	}
	else if (presentState == 4) {
		general(1.0f, angle);
		drawCharPattern1(presentScale);
	}
	else if (presentState == 5) {
		//timeLenght = 10.0f;
		scaleWithOrder(51*51);
	}
	else if (presentState == 6) {
		timeLenght = 5.0f;
		decreaseFromLeft(0, 1.0f);
	}
	else if (presentState == 7) {
		rotationPattern(53, 90);
	}
	else if (presentState == 8) {
		scaleWithOrder(26*26);
	}
	else if (presentState == 9) {
		decreaseFromLeft(0, 0.5f);
	}
	else if (presentState == 10) {

	}

	gl::drawString(toString(presentState), Vec2f(10, 10), ColorA(0.3f, 0.3f, 0.7f, 1.0f), Font("Arial", 30));
	// all debugging things 
	//gl::drawString(toString(presentScale*rectSize*units_w), Vec2f(10, 10), ColorA(0.3f, 0.3f, 0.7f, 1.0f), Font("Arial", 30));
	if (debug_) {
		// draw fps
		gl::drawString(toString(getAverageFps()), Vec2f(10, 10), ColorA(0.3f, 0.3f, 0.7f, 1.0f), Font("Arial", 30));
	}

	if (paramsOn_) {

		config_.drawParams();
	}

	// draw all shortcut keys to dialog
	shortcutKey_.draw();
}

CINDER_APP_NATIVE(MahanakhonParkingTestApp, RendererGl)
