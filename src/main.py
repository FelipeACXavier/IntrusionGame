import os
import json
import time
import pygame
import argparse
import settings

from sys import exit
from level import Level
from screeninfo import get_monitors

parser = argparse.ArgumentParser(
                    prog='IntrusionGame',
                    description="""Simulates a specified intrusion game based on the \"Physical Intrusion
                                   Games—Optimizing Surveillance by Simulation and Game Theory\" paper""",
                    epilog='For research purpose only, see: https://github.com/FelipeACXavier/IntrusionGame')

parser.add_argument('-c', dest="config", help="Specify the simulation config")
parser.add_argument('-d', dest="directory", help="Run all configs in directory")
parser.add_argument('-i', dest="iterations", help="Overwrite config iterations")
parser.add_argument('-r', dest="runs", default=1, help="How many runs to average over")
parser.add_argument('--fps', dest="fps", help="Overwrite config fps")
parser.add_argument('--cycles', dest="cycles", help="Overwrite config cycles per frame")

class Game:
    def __init__(self, config_file, stats):
        self.name= str()
        self.simulation_time = int()
        self.setup(config_file)

        self.font = pygame.font.SysFont("Arial", 18)

        self.screen = pygame.display.set_mode((settings.WIDTH, settings.HEIGHT))
        self.clock = pygame.time.Clock()

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
        if len(monitors) > 1 and False:
            pos_x = monitors[0].width + (monitors[1].width - settings.WIDTH) / 2
            pos_y = (monitors[1].height - settings.WIDTH) / 2
        else:
            pos_x = (monitors[0].width - settings.WIDTH) / 2
            pos_y = (monitors[0].height - settings.WIDTH) / 2

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
        print("Took {:.4f} minutes to {}".format(self.time_taken / 60, "win" if success else "lose"))

    def is_day_done(self):
        return self.time_taken == self.simulation_time

    def is_stopped(self):
        return self.stop

    def run(self):
        while self.run_game:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.run_game = False
                    self.stop = True
                    return

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

            self.update_stats()
            pygame.display.update()
            self.clock.tick(settings.FPS)

class SimulationStats:
    def __init__(self, test_type, iterations):
        self.wins = 0
        self.losses = 0

        self.iterations = iterations

        self.successes = 0
        self.failures = 0
        self.sojurn_time = 0
        self.test_type = test_type

        self.win_time = 0
        self.loss_time = 0
        self.avg_win_time = 0
        self.avg_loss_time = 0

        self.start = time.time()

    def update_result(self, result, iteration):
        if result[0]:
            self.wins += 1
            self.win_time += result[1]
        else:
            self.losses += 1
            self.loss_time += result[1]

        self.successes += result[2]["success"]
        self.failures += result[2]["failure"]

        self.sojurn_time += (result[1] / settings.DAY_LENGTH)
        self.avg_win_time = self.win_time / (iteration + 1)
        self.avg_loss_time = self.loss_time / (iteration + 1)

        if self.test_type == "p-test":
            print("Attempt {} has p={:.4f}".format(iteration, self.p_value()))
        elif self.test_type == "q-test":
            print("Attempt {} has q={:.4f}".format(iteration, self.q_value(iteration + 1)))

    def done(self):
        end = time.time()
        print("Done running for {:.4f} seconds".format(end - self.start))
        self.dump()

    def dump(self):
        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        print("Done with {} iterations".format(self.iterations))
        print("The attacker won {} games and lost {}".format(self.wins, self.losses))
        print("Average win time {:.4f} minutes".format(self.avg_win_time))
        print("Average loss time {:.4f} minutes".format(self.avg_loss_time))
        print("Calculated p value = {:.6f}".format(self.p_value()))
        print("Calculated q value = {:.6f}".format(self.q_value(self.iterations)))
        print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")

    def p_value(self):
        if self.successes == 0 and self.failures == 0:
            return 0

        return self.successes / (self.successes + self.failures)

    def q_value(self, iteration):
        return self.sojurn_time / iteration

    def save(self, file):
        print("Saving results to " + file)
        results = {
            "wins": self.wins,
            "losses": self.losses,
            "avg_win_time": self.avg_win_time,
            "avg_loss_time": self.avg_loss_time,
            "p-value": self.p_value(),
            "successes": self.successes,
            "failures": self.failures,
            "q-value": self.q_value(self.iterations),
            "sojurn_time": self.sojurn_time,
            "iterations": self.iterations,
            "test_type": self.test_type
        }

        with open(file, "w") as f:
            json.dump(results, f, indent=2)

def run_simulation(config_file, runs=None, fps=None, cycles=None):
    print("Running with file: {}".format(config_file))

    test_type = str()
    iterations = int()
    with open(config_file, 'r') as config:
        data = json.load(config)
        test_type = data["test_type"]
        iterations = data["iterations"] if runs == None else int(runs)

        settings.FPS = data["fps"] if fps == None else int(fps)
        settings.DEBUG = data["log_debug"]
        settings.DAY_LENGTH = data["day_duration"] * 60
        settings.CYCLES_PER_FRAME = data["cycles_per_frame"] if cycles == None else int(cycles)

    # Always init the library before the simulation starts
    pygame.init()

    stats = SimulationStats(test_type, iterations)
    game = Game(config_file, stats)
    for i in range(iterations):
        game.run()

        stats.update_result(game.result(), i)

        if game.is_stopped():
            break

        game.reset(i + 1, config_file, stats)

    stats.done()
    stats.save("results/{}".format(os.path.basename(config_file)))

    # Dont forget to deinitialise the library
    pygame.quit()

    return stats.p_value(), stats.q_value(iterations)

if __name__ == "__main__":
    args = parser.parse_args()

    p = 0
    q = 0
    runs = int(args.runs)

    if args.config != None:
        for i in range(runs):
            pi, qi = run_simulation(args.config, runs=args.iterations, fps=args.fps, cycles=args.cycles)
            p += pi
            q += qi

    elif args.directory != None:
        for filename in os.listdir(args.directory):
            config_file = os.path.join(args.directory, filename)
            # Checking if it is a file
            if not os.path.isfile(config_file):
                continue

            run_simulation(config_file, runs=args.iterations, fps=args.fps, cycles=args.cycles)
    else:
        print("No configuration file provided")

    print("p={:.6f}, q={:6f}".format(p / runs, q / runs))
    exit()