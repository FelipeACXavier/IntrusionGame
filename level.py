import json
import pygame
import settings

from employee import Employee
from guard import Guard
from door import Door
from attacker import Attacker

class Level:
  def __init__(self, name, area):
    self.has_player = False
    self.level_name = name
    self.areas = []

    self.add_area(area)

  def name(self):
     return self.level_name

  def add_area(self, area):
     print("Adding area: {}x{}".format(area["w"], area["h"]))

  def update(self):
    pass

class Building:
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
        self.create_guards(data["guards"])

    def create_attacker(self, config, done_callback):
       self.attacker = Attacker(
          pygame.math.Vector2(config["x"], config["y"]),
          config["attack_period"],
          config["stay_period"],
          config["behaviour"],
          self.doors,
          done_callback)

    def create_employees(self, config):
       for index, employee in enumerate(config):
          self.employees.append(
             Employee(index,
                      pygame.math.Vector2(employee["x"], employee["y"]),
                      employee["maximum_stay_time"],
                      employee["minimum_stay_time"]))

    def create_doors(self, config):
        for index, door in enumerate(config):
          self.doors.append(
             Door(index,
                  pygame.math.Vector2(door["x"], door["y"]),
                  door["type"],
                  door["direction"],
                  door["inter_opening_time"],
                  door["short_opening_probability"],
                  door["maximum_open_time"],
                  door["maximum_close_time"]))

    def create_guards(self, config):
      for index, guard in enumerate(config):
          self.guards.append(
             Guard(index,
                   pygame.math.Vector2(guard["x"], guard["y"]),
                   guard["check_radius"],
                   guard["max_check_time"],
                   guard["min_check_time"],
                   self.employees,
                   self.attacker))

    def run(self):
      for e in self.employees:
        e.update()
      for g in self.guards:
        g.update()
      for d in self.doors:
        d.update()

      self.attacker.update()
