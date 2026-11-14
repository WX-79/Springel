#include "raylib.h"
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include "resource_dir.h"

enum GameState {
	STATE_MENU = 0,
	STATE_GAME,
	STATE_GAME_OVER,
	STATE_ACTION_2,
	STATE_ACTION_3
};

enum MusicTrack {
	MUSIC_NONE = 0,
	MUSIC_MENU,
	MUSIC_GAME
};

enum Difficulty {
	DIFFICULTY_EASY = 0,
	DIFFICULTY_MEDIUM,
	DIFFICULTY_HARD
};

struct DifficultySettings {
	float start_speed;
	float min_speed;
	float max_speed;
	float gravity;
	float bounce_velocity;
	int box_growth;
	int max_box_height;
};

static const DifficultySettings difficulty_settings[3] = {
	{ 0.9f, 0.7f, 2.6f, 0.08f, -3.4f, 10, 38 },
	{ 1.2f, 1.0f, 3.4f, 0.10f, -3.8f, 18, 42 },
	{ 1.6f, 1.2f, 4.2f, 0.13f, -4.2f, 30, 46 }
};

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	bool debug = false;
	const int virtualWidth = 128;
	const int virtualHeight = 128;
	const Color background_color = { 0, 206, 209, 255 };
	const Color button_color = { 0, 206, 209, 255 };
	const Color button_option_color = { 0, 206, 209, 100 };

	GameState current_state = STATE_MENU;
	int selected_menu_item = 0;
	int selected_game_over_item = 0;

	float character_y = 50.0f;
	float character_x = 50.0f;
	bool presed = false;
	uint8_t bounce_direction = 1;
	Texture texture_curent;
	float bounce_velocity = -3.8f;
	float bounce_velocity_std = -3.8f;

	float velocity_y = 0.0f;
	float gravity = 0.10f;
	float min_world_speed = 0.6f;
	float max_world_speed = 1.5f;
	int box_growth = 18;
	int max_box_height = 42;
	Difficulty current_difficulty = DIFFICULTY_MEDIUM;

	bool touching_ground = false;
	float world_speed_x = 1.0f;
	float speed_accel = 0.1f;
	float cloud_x = 10;
	float cloud_y = 10;
	bool colidet = false;

	float boxDOWN_w = 15;
	float boxDOWN_h = -40;
	float boxDOWN_x = 160;
	float boxDOWN_y = 128;

	float boxUP_w = 15;
	float boxUP_h = 15;
	float boxUP_x = boxDOWN_x;
	float boxUP_y = 0;

	bool close_game = false;
	bool game_close_now = false;

	Rectangle boxUP = { boxUP_x, boxUP_y, boxUP_w, boxUP_h };
	Rectangle boxDOWN = { boxDOWN_x, boxDOWN_y + boxDOWN_h, boxDOWN_w, -boxDOWN_h };
	Rectangle box_character = { character_x, character_y, 16, 16 };
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
	InitWindow(virtualWidth, virtualHeight, "Springel");
	ToggleFullscreen();

	RenderTexture2D target = LoadRenderTexture(virtualWidth, virtualHeight);
	SearchAndSetResourceDir("resources");

	Texture jumper_norm0 = LoadTexture("jumper_norm_0.png");
	Texture jumper_norm1 = LoadTexture("jumper_norm_1.png");
	Texture jumper_norm2 = LoadTexture("jumper_norm_2.png");
	Texture jumper_presed0 = LoadTexture("jumper_presed_0.png");
	Texture jumper_presed1 = LoadTexture("jumper_presed_1.png");
	Texture jumper_presed2 = LoadTexture("jumper_presed_2.png");
	Texture cloud = LoadTexture("cloud.png");

	InitAudioDevice();
	bool audio_ready = IsAudioDeviceReady();

	Music menu_music = { 0 };
	Music game_music = { 0 };
	Sound game_over_sound = { 0 };
	bool has_menu_music = false;
	bool has_game_music = false;
	bool has_game_over_sound = false;
	MusicTrack current_music = MUSIC_NONE;

	if (audio_ready)
	{
		SetMasterVolume(0.7f);

		if (FileExists("menu_music.mp3"))
		{
			menu_music = LoadMusicStream("menu_music.mp3");
			has_menu_music = IsMusicValid(menu_music);
			if (has_menu_music)
			{
				menu_music.looping = true;
				SetMusicVolume(menu_music, 0.6f);
			}
		}

		if (FileExists("game_music.mp3"))
		{
			game_music = LoadMusicStream("game_music.mp3");
			has_game_music = IsMusicValid(game_music);
			if (has_game_music)
			{
				game_music.looping = true;
				SetMusicVolume(game_music, 0.6f);
			}
		}

		if (FileExists("game_over.mp3"))
		{
			game_over_sound = LoadSound("game_over.mp3");
			has_game_over_sound = IsSoundValid(game_over_sound);
			if (has_game_over_sound)
			{
				SetSoundVolume(game_over_sound, 0.8f);
			}
		}
	}

	auto start_game = [&](Difficulty difficulty) {
		const DifficultySettings &cfg = difficulty_settings[difficulty];
		current_difficulty = difficulty;

		character_y = 50.0f;
		character_x = 50.0f;
		velocity_y = 0.0f;
		gravity = cfg.gravity;
		world_speed_x = cfg.start_speed;
		min_world_speed = cfg.min_speed;
		max_world_speed = cfg.max_speed;
		box_growth = cfg.box_growth;
		max_box_height = cfg.max_box_height;
		bounce_velocity = cfg.bounce_velocity;
		bounce_velocity_std = cfg.bounce_velocity;
		bounce_direction = 1;

		touching_ground = false;
		presed = false;
		colidet = false;

		cloud_x = 10;
		cloud_y = 10;

		boxUP_w = 15;
		boxUP_h = 15;
		boxUP_x = 140;
		boxUP_y = 0;
		boxDOWN_w = 15;
		boxDOWN_h = -40;
		boxDOWN_x = 140;
		boxDOWN_y = 128;

		boxUP = { boxUP_x, boxUP_y, boxUP_w, boxUP_h };
		boxDOWN = { boxDOWN_x, boxDOWN_y + boxDOWN_h, boxDOWN_w, -boxDOWN_h };
		box_character = { character_x + 3, character_y, 8, 16 };

		close_game = false;
		current_state = STATE_GAME;
	};

	SetTargetFPS(60);

	while (!game_close_now)
	{
		if (WindowShouldClose() || close_game == true) {
			game_close_now = true;
		}

		if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

		float screenWidth = (float)GetScreenWidth();
		float screenHeight = (float)GetScreenHeight();

		float scale = (screenWidth / (float)virtualWidth < screenHeight / (float)virtualHeight)
		? (screenWidth / (float)virtualWidth)
		: (screenHeight / (float)virtualHeight);

		float destWidth = (float)virtualWidth * scale;
		float destHeight = (float)virtualHeight * scale;
		float destX = (screenWidth - destWidth) * 0.5f;
		float destY = (screenHeight - destHeight) * 0.5f;

		if (current_music == MUSIC_MENU && has_menu_music) UpdateMusicStream(menu_music);
		else if (current_music == MUSIC_GAME && has_game_music) UpdateMusicStream(game_music);

		BeginTextureMode(target);

		ClearBackground(BLACK);

		if (current_state == STATE_MENU)
		{
			if (current_music != MUSIC_MENU && has_menu_music && !(has_game_over_sound && IsSoundPlaying(game_over_sound))) {
				if (current_music == MUSIC_GAME && has_game_music) StopMusicStream(game_music);
				PlayMusicStream(menu_music);
				current_music = MUSIC_MENU;
			}

			if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
				selected_menu_item = (selected_menu_item + 1) % 4;
			}
			if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
				selected_menu_item = (selected_menu_item + 3) % 4;
			}
			if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) selected_menu_item = 0;
			if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) selected_menu_item = 1;
			if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) selected_menu_item = 2;
			if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) selected_menu_item = 3;

			ClearBackground(background_color);

			DrawText("Springel", 20, 10, 10, WHITE);

			Rectangle btn1 = { -2, 30, 132, 18 };
			Rectangle btn2 = { -2, 52, 132, 18 };
			Rectangle btn3 = { -2, 74, 132, 18 };
			Rectangle btn4 = { -2, 96, 132, 18 };

			Color col1 = (selected_menu_item == 0) ? button_color : button_option_color;
			Color col2 = (selected_menu_item == 1) ? button_color : button_option_color;
			Color col3 = (selected_menu_item == 2) ? button_color : button_option_color;
			Color col4 = (selected_menu_item == 3) ? button_color : button_option_color;

			DrawRectangleRec(btn1, col1);
			DrawRectangleRec(btn2, col2);
			DrawRectangleRec(btn3, col3);
			DrawRectangleRec(btn4, col4);

			DrawText(" 1. Start Game EASY", (int)btn1.x + 2, (int)btn1.y + 5, 8, BLACK);
			DrawText(" 2. Start Game MIDLE",   (int)btn2.x + 2, (int)btn2.y + 5, 8, BLACK);
			DrawText(" 3. Start Game HARD",   (int)btn3.x + 2, (int)btn3.y + 5, 8, BLACK);
			DrawText(" 4. Quit",       (int)btn4.x + 2, (int)btn4.y + 5, 8, BLACK);

			if (IsKeyPressed(KEY_ENTER)) {
				if (selected_menu_item == 0) {
					start_game(DIFFICULTY_EASY);
				}
				else if (selected_menu_item == 1) {
					start_game(DIFFICULTY_MEDIUM);
				}
				else if (selected_menu_item == 2) {
					start_game(DIFFICULTY_HARD);
				}
				else if (selected_menu_item == 3) {
					game_close_now = true;
				}
			}
		}
		else if (current_state == STATE_GAME)
		{
			if (current_music != MUSIC_GAME) {
				if (current_music == MUSIC_MENU && has_menu_music) StopMusicStream(menu_music);
				if (has_game_music) {
					PlayMusicStream(game_music);
					current_music = MUSIC_GAME;
				} else {
					current_music = MUSIC_NONE;
				}
				if (has_game_over_sound && IsSoundPlaying(game_over_sound)) StopSound(game_over_sound);
			}

			ClearBackground(background_color);

			boxUP = { boxUP_x, boxUP_y, boxUP_w, boxUP_h };
			boxDOWN = { boxDOWN_x, boxDOWN_y + boxDOWN_h, boxDOWN_w, -boxDOWN_h };

			if (boxUP_x < -20) {
				boxUP_x = 128;
				boxUP_h += rand() % box_growth;
				boxDOWN_h -= rand() % box_growth;
				if (boxUP_h > max_box_height) boxUP_h = max_box_height;
				if (boxDOWN_h < -max_box_height) boxDOWN_h = -max_box_height;
			}

			bounce_direction = 1;
			box_character = { character_x + 3, character_y, 8, 16 };

			if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) bounce_direction = 2;
			if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) bounce_direction = 0;

			presed = false;

			velocity_y += gravity;
			character_y += velocity_y;

			if (character_y <= 16) {
				character_y = 16;
				touching_ground = true;
				velocity_y = 0.5f;
			} else if (character_y >= 112) {
				character_y = 112;
				touching_ground = true;
				velocity_y = bounce_velocity;
			} else {
				touching_ground = false;
			}

			if (character_x > 112) character_x = 112;
			if (character_x < 16) character_x = 16;
			if (character_y < 20) {
				presed = true;
			}

			if (bounce_direction == 2) {
				if (!presed) {
					texture_curent = jumper_norm2;

				} else {
					texture_curent = jumper_presed2;
				}
			}
			else if (bounce_direction == 0) {
				if (!presed) {
					texture_curent = jumper_norm0;

				} else {
					texture_curent = jumper_presed0;
				}
			}
			else {
				if (!presed) {
					texture_curent = jumper_norm1;

				} else {
					texture_curent = jumper_presed1;
				}
			}

			if (bounce_direction == 0) {
				world_speed_x += speed_accel;
			} else if (bounce_direction == 2) {
				world_speed_x -= speed_accel;
			}

			cloud_x -= world_speed_x / 4;
			boxUP_x -= world_speed_x / 4;
			boxDOWN_x = boxUP_x;

			if (cloud_x < -32) {
				cloud_x = 128;
			}

			if (world_speed_x < min_world_speed) { world_speed_x = min_world_speed; }
			if (world_speed_x > max_world_speed) { world_speed_x = max_world_speed; }

			colidet = CheckCollisionRecs(boxUP, box_character) || CheckCollisionRecs(boxDOWN, box_character);

			if (colidet) {
				if (current_music == MUSIC_GAME && has_game_music) StopMusicStream(game_music);
				current_music = MUSIC_NONE;
				if (has_game_over_sound) PlaySound(game_over_sound);
				selected_game_over_item = 0;
				current_state = STATE_GAME_OVER;
			}

			if (debug == true) {
				DrawRectangleRec(box_character, GOLD);
			}
			DrawRectangleRec(boxUP, DARKGRAY);
			DrawRectangleRec(boxDOWN, DARKGRAY);
			DrawTexture(texture_curent, (int)character_x, (int)character_y, WHITE);
			DrawTexture(cloud, (int)cloud_x, (int)cloud_y, WHITE);
		}
		else if (current_state == STATE_GAME_OVER)
		{
			if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
				selected_game_over_item = (selected_game_over_item + 1) % 2;
			}
			if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
				selected_game_over_item = (selected_game_over_item + 1) % 2;
			}

			ClearBackground(background_color);

			DrawText("Game Over", 26, 14, 14, WHITE);

			Rectangle go1 = { -2, 48, 132, 18 };
			Rectangle go2 = { -2, 72, 132, 18 };
			Color gcol1 = (selected_game_over_item == 0) ? WHITE : button_option_color;
			Color gcol2 = (selected_game_over_item == 1) ? WHITE : button_option_color;

			DrawRectangleRec(go1, gcol1);
			DrawRectangleRec(go2, gcol2);
			DrawText(" 1. New Game", (int)go1.x + 2, (int)go1.y + 5, 8, BLACK);
			DrawText(" 2. Main Menu", (int)go2.x + 2, (int)go2.y + 5, 8, BLACK);
			DrawText("1/R Neu   2/M Menue", 8, 100, 6, BLACK);

			bool new_game = IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1);
			bool main_menu = IsKeyPressed(KEY_M) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2);

			if (IsKeyPressed(KEY_ENTER)) {
				if (selected_game_over_item == 0) new_game = true;
				else main_menu = true;
			}

			if (new_game) {
				start_game(current_difficulty);
			}
			else if (main_menu) {
				current_state = STATE_MENU;
			}
		}


		EndTextureMode();

		BeginDrawing();
		ClearBackground(BLACK);

		Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height };
		Rectangle destRec = { destX, destY, destWidth, destHeight };
		Vector2 origin = { 0.0f, 0.0f };

		DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);

		EndDrawing();
	}

	UnloadRenderTexture(target);
	UnloadTexture(jumper_norm0);
	UnloadTexture(jumper_norm1);
	UnloadTexture(jumper_norm2);
	UnloadTexture(jumper_presed0);
	UnloadTexture(jumper_presed1);
	UnloadTexture(jumper_presed2);
	UnloadTexture(cloud);

	if (audio_ready)
	{
		if (has_menu_music) UnloadMusicStream(menu_music);
		if (has_game_music) UnloadMusicStream(game_music);
		if (has_game_over_sound) UnloadSound(game_over_sound);
		CloseAudioDevice();
	}

	CloseWindow();

	return 0;
}
