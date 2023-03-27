import math
import json
import pygame
import random
import settings

from entity import Entity

class Guard(Entity):
  def __init__(self, id, config, day_duration, entities):
    random.seed()

    attacker = entities[-1]
    x = random.randint(0, settings.WIDTH)
    y = random.randint(0, settings.HEIGHT)

    behaviour = config["behaviour"]
    if behaviour != "reset" and behaviour != "stroll":
      raise Exception("Guard behaviour {} is not valid".format(behaviour))

    super().__init__(pygame.math.Vector2(x, y), (255, 0, 0))

    self.initial_pos = pygame.math.Vector2(x, y)

    self.name = id
    self.check_time = 0
    self.max_check_time = config["max_check_time"] * 60
    self.min_check_time = config["min_check_time"] * 60
    self.show_radius = settings.TILE_SIZE * config["check_radius"]
    self.check_radius = (self.show_radius)**2

    duration_in_ticks = day_duration * 60 * 60

    self.mission = 0
    self.in_mission = False
    self.mission_period = duration_in_ticks / config["number_of_missions"]
    self.max_mission_time = config["max_mission_time"] * 60
    self.min_mission_time = config["min_mission_time"] * 60

    self.wait_for_mission = random.uniform(0, self.mission_period)

    self.entities = entities
    self.checking_entity = None

    self.behaviour = behaviour

    self.stroll_speed = config["stroll_speed"]
    self.speed = self.stroll_speed * settings.TILE_SIZE

    if settings.DEBUG:
      print("Created guard at ({}, {}) and {}".format(self.pos.x, self.pos.y, self.check_probability))


  def perform_check(self):
    if self.checking_entity != None:
      self.checking_entity = None
      return

    possible_checks = list()
    for entity in self.entities:
      if entity.is_being_checked():
        continue

      dist = (self.x() - entity.x())**2 + (self.y() - entity.y())**2
      if dist <= self.check_radius:
        possible_checks.append(entity)

    if len(possible_checks) > 0:
      self.checking_entity = random.choice(possible_checks)

      self.start_check()
      self.checking_entity.start_check()
      self.check_time = random.uniform(self.min_check_time, self.max_check_time)

  def start_mission(self):
    if not self.in_mission:
      self.in_mission = True
      self.speed = 0.5 * settings.TILE_SIZE
      self.mission = random.uniform(self.min_mission_time, self.max_mission_time)

  def stop_mission(self):
    if self.in_mission:
      self.in_mission = False
      self.speed = self.stroll_speed * settings.TILE_SIZE
      self.wait_for_mission = self.mission_period

      if self.behaviour == "reset":
        self.reset_pos()

  def reset_pos(self):
    self.pos.x = self.initial_pos.x
    self.pos.y = self.initial_pos.y
    self.constrain(self.speed)

  def move(self, speed):
    if self.wait_for_mission > 0:
      self.wait_for_mission -= 1

      if self.behaviour == "reset":
        return
    else:
      self.start_mission()

    if self.mission > 0:
      self.mission -= 1
    else:
      self.stop_mission()

    if self.check_time > 0:
      self.check_time -= 1
      return

    if self.is_checking:
      self.stop_check()

      if self.checking_entity != None:
        self.checking_entity.stop_check(self.check_radius)

    super().move(self.speed)

    if self.in_mission:
      self.perform_check()

  def update(self):
    super().update()

    # Draw check radius
    pygame.draw.circle(self.surface, self.color if self.in_mission else (255, 255, 255), self.pos, self.show_radius, 1)


