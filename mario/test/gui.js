SDL.init();

Font.init();
font = Font.open("test/extra/wqy.ttc", 32);

mode = SDL.getDisplayMode();
Debug.dump(mode);

w = SDL.createWindow("hello", 10, 10, 800, 600, false);
canvas = w.getCanvas();

texImage = Image.loadTexture(canvas, "test/extra/test.png");
texFont = font.genTexture(canvas, "Hello, world(中文)!", 0xFF00FF00);

i = 0; 
while(true) {
	if(i >= 1500) {
		canvas.setColor(0x0);
		canvas.clear();
		canvas.filledRoundedRectangle(110, 110, 200, 200, 6, 0xFFFF0000);
		canvas.filledRectangle(10, 10, 100, 100, 0xFFFF0000);
		canvas.aaline(100, 200, 200, 400, 0xFF00FF00);

		x = Math.randInt(100, 300);
		y = Math.randInt(100, 300);

		canvas.copyTexture(texImage, 
				new Rect(0, 0, texImage.w, texImage.h), 
				new Rect(x, y, texImage.w/2, texImage.h/2));
		canvas.copyTexture(texFont, 
				new Rect(0, 0, texFont.w, texFont.h),
				new Rect(x, y+100, texFont.w, texFont.h));

		canvas.drawText("中文.^()*)!~", 100, 400, font, 0xFF0000FF);

		canvas.refresh();
		i = 0;
	}
	else {
		i++;
	}

	ev = Event.pollEvent();
	if(ev.type == Event.QUIT)
		break;

	if(ev.type == Event.KEY_UP) {
		if(ev.keyboard.code == 27)
			break;
	}
	System.usleep(50);
}


texImage.destroy();
texFont.destroy();
canvas.destroy();
w.destroy();

font.close();
Font.quit();

SDL.quit();
