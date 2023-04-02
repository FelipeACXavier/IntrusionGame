import os
import json
import pygame
import settings

from level import Level
from screeninfo import get_monitors

class Game:
    def __init__(self, config_file, stats, hidden):
        self.name= str()
        self.simulation_time = int()
        self.setup(config_file)

        self.font = pygame.font.SysFont("JetBrains Mono NL", 18)

        flags = pygame.HIDDEN if hidden or settings.FPS > 120 else pygame.SHOWN
        self.screen = pygame.display.set_mode((settings.WIDTH, settings.HEIGHT), flags)

        self.clock = pygame.time.Clock()

        icon = pygame.image.load('../assets/icon.png')

        pygame.display.set_icon(icon)
        pygame.display.set_caption("Intrusion game")

        self.reset(0, config_file, stats)

    def reset(self, id, config_file, stats):
        self.id = id
        self.wins = stats.wins
        self.losses = stats.losses
        self.win_time = stats.avg_win_time
        self.loss_time = stats.avg_loss_time

        self.successes = 0
        self.time_taken = 0
        self.run_game = True
        self.game_result = False
        self.stop = False

        self.level = Level(config_file, self.game_finished)

    def setup(self, config_file):
        with open(config_file, 'r') as config:
            data = json.load(config)
            self.name = data["level"]
            self.simulation_time = data["day_duration"] * 60 * 60

            settings.TILE_SIZE = data["tile_size"]
            settings.HALF_TILE = settings.TILE_SIZE / 2

            settings.WIDTH = data["width"] * settings.TILE_SIZE
            settings.HEIGHT = data["height"] * settings.TILE_SIZE

        self.setup_display()

    def setup_display(self):
        monitors = get_monitors()
        if len(monitors) > 1:
            pos_x = monitors[0].width + (monitors[1].width - settings.WIDTH) * 1.0
            pos_y = (monitors[1].height - settings.WIDTH) * 0.2
        else:
            pos_x = (monitors[0].width - settings.WIDTH) * 1.0
            pos_y = (monitors[0].height - settings.WIDTH) * 0.2

        os.environ['SDL_VIDEO_WINDOW_POS'] = "%d,%d" % (pos_x, pos_y)

    def update_stats(self):
        text = list()
        text.append("Run: " + str(self.id))
        text.append("Wins: " + str(self.wins) + " Loses: " + str(self.losses))
        text.append("Game value: {:.2f}".format(self.wins / self.losses if self.losses else 1.0))
        text.append("Avg win time: {:.2f}".format(self.win_time))
        text.append("Avg loss time: {:.2f}".format(self.loss_time))
        text.append("fps: " + str(int(self.clock.get_fps())))
        text.append("{:.4f}".format((self.simulation_time - self.time_taken) / 60))

        for i, t in enumerate(text):
            render = self.font.render(t, 1, pygame.Color("coral"))
            self.screen.blit(render, (10, 10 + 20 * i))

    def result(self):
        return (self.game_result, self.time_taken / 60, self.level.result())

    def game_finished(self, success):
        self.run_game = False
        self.game_result = success
        # print("Took {:.4f} minutes to {}".format(self.time_taken / 60, "win" if success else "lose"))

    def is_day_done(self):
        return self.time_taken == self.simulation_time

    def is_stopped(self):
        return self.stop

    def run(self):
        while self.run_game:
            for event in pygame.event.get():
                if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE):
                    self.run_game = False
                    self.stop = True
                    return

            if settings.FPS <= 120:
                self.screen.fill("Black")

            for i in range(settings.CYCLES_PER_FRAME):
                if self.is_day_done():
                    self.game_finished(True)
                    break

                if not self.run_game:
                    return

                # Draw all elements
                self.level.run()
                self.time_taken += 1

            if settings.FPS <= 120:
                self.update_stats()
                pygame.display.update()
                self.clock.tick(settings.FPS)
