import pygame
import random
import settings

from entity import Entity

class Employee(Entity):
    def __init__(self, id, pos, config):
        random.seed()

        behaviour = config["behaviour"]
        if behaviour != "stay" and behaviour != "walk":
          raise Exception("Attacker behaviour {} is not valid".format(behaviour))

        pos.x = random.randint(0, settings.WIDTH)
        pos.y = random.randint(0, settings.HEIGHT)
        super().__init__(pos, (117, random.randint(100, 200), random.randint(0, 50)))

        self.name = id
        self.max_stay_time = config["max_stay_time"]
        self.min_stay_time = config["min_stay_time"]

        self.stay_time = 0
        self.behaviour = behaviour
        self.speed = config["speed"] * settings.TILE_SIZE
        if settings.DEBUG:
            print("Created employee at ({}, {})".format(pos.x, pos.y))

    def stop_check(self, check_radius = None):
        super().stop_check()
        # Move away from the guard check radius
        if self.behaviour == "walk":
            self.adjust_pos(check_radius)
        # self.stay_time = 0

    def constrain(self, speed):
        if self.pos.x >= settings.WIDTH:
            self.pos.x = settings.WIDTH - settings.HALF_TILE
        elif self.pos.x <= 0:
            self.pos.x = settings.HALF_TILE

        if self.pos.y >= settings.HEIGHT:
            self.pos.y = settings.HEIGHT - settings.HALF_TILE
        elif self.pos.y <= 0:
            self.pos.y = settings.HALF_TILE

    def move(self, speed):
        if self.stay_time > 0:
            self.stay_time -= 1
            return

        super().move(speed)

        # Uniform distribution between 10 and 60 minutes
        # self.stay_time = random.uniform(self.min_stay_time, self.max_stay_time)
        self.stay_time = random.uniform(self.min_stay_time, self.max_stay_time) * 60