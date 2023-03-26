import pygame
import random
import settings

from entity import Entity

class Attacker(Entity):
  def __init__(self, pos, attack_period, stay_period, behaviour, doors, done_callback):
    super().__init__(pos, (10, 73, 200))

    if behaviour != "jump" and behaviour != "wait":
      raise Exception("Attacker behaviour {} is not valid".format(behaviour))

    self.door = None
    self.doors = doors

    self.behaviour = behaviour
    self.staying = False
    self.can_attack = True

    self.stay_speed = 5 * settings.TILE_SIZE

    self.stay_period = stay_period * 60
    self.attack_period = attack_period * 60
    self.stay_time = 0
    self.wait_time = 0

    self.done_callback = done_callback
    if settings.DEBUG:
      print("Created attacker at ({}, {})".format(pos.x, pos.y))

  def start_check(self):
    print("Attacker was caught")
    if self.done_callback:
      self.done_callback(False)

  def select_door(self):
    if self.door != None:
      return

    self.door = random.choice(self.doors)
    if settings.DEBUG:
      print("Selected door {}: ({},{})".format(self.door.id(), self.door.x(), self.door.y()))


  def move(self, speed):
    # Count time until we try to attack
    if self.wait_time > 0:
      self.wait_time -= 1
    else:
      self.can_attack = True
      self.staying = False

    if self.stay_time > 0:
      self.stay_time -= 1
      return

    self.select_door()

    if self.door and self.can_attack:
      d_x = self.door.x() - self.x()
      d_y = self.door.y() - self.y()
      if settings.DEBUG:
        print("Distance: ({},{})".format(d_x, d_y))

      if (abs(d_x) > settings.HALF_TILE or abs(d_y) > settings.HALF_TILE):
        if self.behaviour == "walk":
          self.direction.x = d_x
          self.direction.y = d_y
          self.direction = self.direction.normalize()

          self.pos.x += self.direction.x * speed
          self.pos.y += self.direction.y * speed
        elif self.behaviour == "jump":
          self.pos.x = self.door.x()
          self.pos.y = self.door.y()
        else:
          raise Exception("Unknown attacker behaviour")
      else:
        if self.door.is_open() and self.door.to_next_level():
          print("Door open, attack succeeded")
          if self.done_callback:
            self.done_callback(True)

        else:
          if settings.DEBUG:
            print("Door is closed, trying again in {} seconds".format(self.attack_period))
          self.door = None
          self.staying = True

          # Wait for next attempt in a few minutes
          self.wait_time = self.attack_period
          self.can_attack = False

    else:
      super().move(self.stay_speed)

    # Stays in location for some time
    if self.staying:
      self.stay_time = self.stay_period
      if settings.DEBUG:
        print("Moved, now staying for {} minutes".format(self.stay_time / 60))