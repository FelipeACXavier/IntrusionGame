import pygame
import random
import settings

from entity import Entity

class Attacker(Entity):
  def __init__(self, config, doors, done_callback):
    random.seed()

    x = config["x"] + random.randint(0, config["randomization"])
    y = config["y"] + random.randint(0, config["randomization"])

    behaviour = config["behaviour"]
    strategy = config["strategy"]

    super().__init__(pygame.math.Vector2(x, y), (10, 73, 200))

    if behaviour != "jump" and behaviour != "walk":
      raise Exception("Attacker behaviour {} is not valid".format(behaviour))

    if strategy != "p-test" and strategy != "q-test" and strategy != "normal":
      raise Exception("Attacker strategy {} is not valid".format(strategy))

    self.door = None
    self.doors = doors

    self.staying = True
    self.can_attack = False
    self.strategy = strategy
    self.behaviour = behaviour

    self.initial_pos = pygame.math.Vector2(x, y)
    self.successes = 0
    self.failures = 0

    self.speed = config["speed"] * settings.TILE_SIZE
    self.attack_speed = config["attack_speed"] if "attack_speed" in config else self.speed

    self.stay_period = config["stay_period"] * 60
    self.attack_period = config["attack_period"] * 60
    self.stay_time = self.stay_period
    self.wait_time = self.attack_period

    self.done_callback = done_callback
    if settings.DEBUG:
      print("Created attacker at {} waiting {} minutes".format(self.pos, self.wait_time / 60))

  def start_check(self):
    if settings.DEBUG:
      print("Attacker was caught")

    if self.done_callback:
      self.done_callback(False)

  def select_door(self):
    if self.door != None or len(self.doors) < 1:
      return

    self.door = random.choice(self.doors)
    if settings.DEBUG:
      print("Selected door {}: ({},{})".format(self.door.id(), self.door.x(), self.door.y()))

  def constrain(self, speed):
      if self.pos.x > settings.WIDTH:
          self.pos.x = settings.WIDTH - settings.HALF_TILE
      elif self.pos.x < 0:
          self.pos.x = settings.HALF_TILE

      if self.pos.y > settings.HEIGHT:
          self.pos.y = settings.HEIGHT - settings.HALF_TILE
      elif self.pos.y < 0:
          self.pos.y = settings.HALF_TILE

  def result(self):
    return {"success": self.successes, "failure": self.failures }

  def reset_pos(self):
    self.pos.x = random.randint(0, settings.WIDTH) # self.initial_pos.x
    self.pos.y = random.randint(0, settings.HEIGHT) # self.initial_pos.y
    self.constrain(self.speed)

    self.door = None
    self.staying = True

    # Wait for next attempt in a few minutes
    self.wait_time = self.attack_period
    self.can_attack = False
    self.stay_time = self.stay_period

  def move(self, speed):
    # Count time until we try to attack
    if self.wait_time > 0:
      self.wait_time -= 1
    elif self.strategy != "q-test":
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

          self.pos.x += self.direction.x * self.attack_speed
          self.pos.y += self.direction.y * self.attack_speed
        elif self.behaviour == "jump":
          self.pos.x = self.door.x()
          self.pos.y = self.door.y()
        else:
          raise Exception("Unknown attacker behaviour")
      else:
        if self.door.is_open() and self.door.to_next_level():
          # For a p test, we only increment the number of successful attempts
          if self.strategy == "p-test":
            self.successes += 1
            self.reset_pos()
          elif self.done_callback:
            self.done_callback(True)

        else:
          if settings.DEBUG:
            print("Door is closed, trying again in {} seconds".format(self.attack_period))

          if self.strategy == "p-test":
            self.failures += 1
            self.reset_pos()

          self.door = None
          self.staying = True

          # Wait for next attempt in a few minutes
          self.wait_time = self.attack_period
          self.can_attack = False

    else:
      super().move(self.speed)

    # Stays in location for some time
    if self.staying:
      self.stay_time = self.stay_period