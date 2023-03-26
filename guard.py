import math
import pygame
import random
import settings

from entity import Entity

class Guard(Entity):
  def __init__(self, id, pos, check_radius, max_check_time, min_check_time, entities, attacker):
    random.seed()

    super().__init__(pos, (255, 0, 0))
    if settings.DEBUG:
      print("Created guard at ({}, {})".format(pos.x, pos.y))

    self.name = id
    self.check_time = 0
    self.max_check_time = max_check_time * 60
    self.min_check_time = min_check_time * 60
    self.attacker = attacker
    self.entities = entities
    self.checking_entity = None
    self.check_radius = (settings.TILE_SIZE * check_radius)**2

  def check_employees(self):
    if self.checking_entity != None:
      self.checking_entity = None
      return

    for entity in self.entities:
      if entity.is_being_checked():
        continue

      dist = (self.x() - entity.x())**2 + (self.y() - entity.y())**2
      if dist > self.check_radius:
        continue

      self.checking_entity = entity
      self.start_check()
      self.checking_entity.start_check()

      self.check_time = random.uniform(self.max_check_time, self.min_check_time)
      break

  def check_attacker(self):
    dist = (self.x() - self.attacker.x())**2 + (self.y() - self.attacker.y())**2
    if dist > self.check_radius:
      return

    self.attacker.start_check()

  def move(self, speed):
    if self.check_time > 0:
        self.check_time -= 1
        return

    if self.is_checking:
      self.stop_check()

      if self.checking_entity != None:
        self.checking_entity.stop_check(self.check_radius)

    super().move(self.speed)

    self.check_attacker()
    self.check_employees()

  def update(self):
    super().update()
    # Draw check radius
    pygame.draw.circle(self.surface, self.color, self.pos, 3.125 * settings.TILE_SIZE, 1)


