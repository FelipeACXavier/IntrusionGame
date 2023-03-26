import pygame
import random
import settings

from entity import Entity

class Employee(Entity):
    def __init__(self, id, pos, max_stay_time, min_stay_time):
        super().__init__(pos, (117, random.randint(100, 200), random.randint(0, 50)))

        self.name = id
        self.max_stay_time = max_stay_time * 60
        self.min_stay_time = min_stay_time * 60

        self.stay_time = 0
        self.speed = 5 * settings.TILE_SIZE
        if settings.DEBUG:
            print("Created employee at ({}, {})".format(pos.x, pos.y))

    def stop_check(self, check_radius):
        super().stop_check()
        # Move away from the guard check radius
        self.adjust_pos(2 * check_radius, 2 * check_radius)
        self.stay_time = 0

    def move(self, speed):
        if self.stay_time > 0:
            self.stay_time -= 1
            return

        super().move(self.speed)

        # Uniform distribution between 10 and 60 minutes
        self.stay_time = random.uniform(self.min_stay_time, self.max_stay_time)
        if settings.DEBUG:
            print("Employee moved, now staying for {} minutes at ({},{})".format(self.stay_time / 60, self.x(), self.y()))