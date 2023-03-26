import os
import json
import pygame
import settings

from sys import exit
from level import Building
from screeninfo import get_monitors


class Game:
    def __init__(self, id, config_file, wins, loses, win_time, loss_time):
        pygame.init()

        self.name= str()
        self.setup(config_file)


        self.screen = pygame.display.set_mode((settings.WIDTH, settings.WIDTH))
        self.clock = pygame.time.Clock()
        self.run_game = True

        pygame.display.set_caption("Intrusion game")

        self.id = id
        self.wins = wins
        self.loses = loses
        self.win_time = win_time
        self.loss_time = loss_time
        self.time_taken = 0
        self.game_result = False
        self.font = pygame.font.SysFont("Arial", 18)

        self.level = Building(config_file, self.game_finished)

    def setup(self, config_file):
        with open(config_file, 'r') as config:
            data = json.load(config)
            self.name = data["level"]
            settings.FPS = data["fps"]
            settings.WIDTH = data["width"]
            settings.HEIGHT = data["height"]
            settings.TILE_SIZE = data["tile_size"]
            settings.HALF_TILE = settings.TILE_SIZE / 2

        self.setup_display()

    def setup_display(self):
        monitors = get_monitors()
        if len(monitors) > 1 and False:
            pos_x = monitors[0].width + (monitors[1].width - settings.WIDTH) / 2
            pos_y = (monitors[1].height - settings.WIDTH) / 2
        else:
            pos_x = (monitors[0].width - settings.WIDTH) / 2
            pos_y = (monitors[0].height - settings.WIDTH) / 2

        os.environ['SDL_VIDEO_WINDOW_POS'] = "%d,%d" % (pos_x, pos_y)

    def update_stats(self):
        renders = []
        id_text = "Run: " + str(self.id)
        stat_text = "Wins: " + str(self.wins) + " Loses: " + str(self.loses)
        value_text = "Game value: {:.2f}".format(self.wins / self.loses if self.loses else 1.0)
        win_text = "Avg win time: {:.2f}".format(self.win_time)
        loss_text = "Avg loss time: {:.2f}".format(self.loss_time)
        fps = "fps: " + str(int(self.clock.get_fps()))

        renders.append(self.font.render(self.name, 1, pygame.Color("coral")))
        renders.append(self.font.render(id_text, 1, pygame.Color("coral")))
        renders.append(self.font.render(stat_text, 1, pygame.Color("coral")))
        renders.append(self.font.render(value_text, 1, pygame.Color("coral")))
        renders.append(self.font.render(win_text, 1, pygame.Color("green")))
        renders.append(self.font.render(loss_text, 1, pygame.Color("red")))
        renders.append(self.font.render(fps, 1, pygame.Color("coral")))

        for i, render in enumerate(renders):
            self.screen.blit(render, (10, 10 + 20 * i))


    def result(self):
        return (self.game_result, self.time_taken / 60)

    def game_finished(self, success):
        self.run_game = False
        self.game_result = success
        print("Took {:.4f} seconds to {}".format(self.time_taken / 60, "win" if success else "lose"))

    def run(self):
        while self.run_game:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    exit()

            # Draw all elements
            self.screen.fill('Black')
            self.level.run()
            self.update_stats()

            pygame.display.update()
            self.time_taken += 1
            self.clock.tick(settings.FPS)

        pygame.quit()


if __name__ == "__main__":
    wins = 0
    win_time = 0
    avg_win_time = 0
    loses = 0
    loss_time = 0
    avg_loss_time = 0
    proportion = 0

    iterations = int()
    level_config = str()

    with open("settings.json", 'r') as config:
        data = json.load(config)
        iterations = data["iterations"]
        level_config = data["level"]
        settings.DEBUG = data["log_debug"]

    for i in range(iterations):
        game = Game(i, level_config, wins, loses, avg_win_time, avg_loss_time)
        game.run()
        result = game.result()
        if result[0]:
            wins += 1
            win_time += result[1]
        else:
            loses += 1
            loss_time += result[1]

        avg_win_time = win_time / (i + 1)
        avg_loss_time = loss_time / (i + 1)

    print("After {} iterations, the attacker won {} games and lost {}".format(iterations, wins, loses))
    print("The time to win was {:.4f} and to lose {:.4f}".format(avg_win_time, avg_loss_time))