import json
import pygame
import random
import settings

from door import Door
from guard import Guard
from attacker import Attacker
from employee import Employee

class Level:
    def __init__(self, config_file, done_callback):
      self.surface = pygame.display.get_surface()

      self.doors = []
      self.guards = []
      self.employees = []

      with open(config_file, 'r') as config:
        data = json.load(config)

        self.create_doors(data["doors"])
        self.create_employees(data["employees"])
        self.create_attacker(data["attacker"], done_callback)
        self.create_guards(data["guards"], data["day_duration"])

    def create_attacker(self, config, done_callback):
       self.attacker = Attacker(config, self.doors, done_callback)

    def create_employees(self, config):
       for index in range(config["number_of_employees"]):
          x = settings.WIDTH / 2 + random.randint(0, config["randomization"])
          y = settings.HEIGHT / 2 + random.randint(0, config["randomization"])

          self.employees.append(Employee(index, pygame.math.Vector2(x, y), config))

    def create_doors(self, config):
        for index, door in enumerate(config):
          self.doors.append(Door(index, door))

    def create_guards(self, config, day_duration):
      for index, guard in enumerate(config):
          self.guards.append(Guard(index, guard, day_duration, self.employees + [self.attacker]))

    def run(self):
      for e in self.employees:
        e.update()
      for g in self.guards:
        g.update()
      for d in self.doors:
        d.update()

      self.attacker.update()
