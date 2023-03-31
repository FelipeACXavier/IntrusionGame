import json
import time
import settings

class GameStats:
    def __init__(self, test_type, iterations):
        self.wins = 0
        self.losses = 0

        self.iterations = iterations

        self.successes = 0
        self.failures = 0
        self.p_samples = list()
        self.q_samples = list()
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

        self.avg_win_time = self.win_time / (iteration + 1)
        self.avg_loss_time = self.loss_time / (iteration + 1)

        self.p_samples.append(self.p_value())
        self.q_samples.append(result[1] / settings.DAY_LENGTH) # Result and DAY_LENGTH are in minutes

        if self.test_type == "p-test":
            print("Attempt {} has p={:.4f}".format(iteration, self.p_value()))
        elif self.test_type == "q-test":
            print("Attempt {} has q={:.4f}".format(iteration, self.q_value()))

    def done(self):
        end = time.time()
        print("=========================================")
        print("{} iterations done after {:.4f} seconds".format(self.iterations, end - self.start))
        print("The attacker won {} games and lost {}".format(self.wins, self.losses))
        print("Average win time {:.4f} minutes".format(self.avg_win_time))
        print("Average loss time {:.4f} minutes".format(self.avg_loss_time))
        print("Calculated p value = {:.6f}".format(self.p_value()))
        print("Calculated q value = {:.6f}".format(self.q_value()))
        print("=========================================")

    def p_value(self):
        if self.successes == 0 and self.failures == 0:
            return 0

        return self.successes / (self.successes + self.failures)

    def q_value(self):
        if len(self.q_samples) < 1:
            return 0

        return sum(self.q_samples) / len(self.q_samples)

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
            "q-value": self.q_value(),
            "iterations": self.iterations,
            "test_type": self.test_type
        }

        with open(file, "w") as f:
            json.dump(results, f, indent=2)
