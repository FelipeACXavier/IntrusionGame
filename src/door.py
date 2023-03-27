import pygame
import random
import settings

DOOR_SIZE = 2

class Door:
  def __init__(self, id, config):
    random.seed()

    door_type = config["type"]
    if door_type != "intra_level" and door_type != "inter_level":
      raise Exception("Door type {} is not valid".format(door_type))

    x = config["x"] * settings.WIDTH
    y = config["y"] * settings.HEIGHT

    self.open = False
    self.name = id
    self.type = door_type
    self.pos = pygame.math.Vector2(x, y)
    self.surface = pygame.display.get_surface()

    [w, h, self.area] = self.create_area(config["direction"])

    self.rect = pygame.Rect(self.pos.x , self.pos.y, w, h)

    self.short_opening = config["short_opening_probability"]
    self.inter_duration = config["inter_opening_time"] * 60
    self.max_open_duration = config["max_open_time"] * 60
    self.min_open_duration = config["min_open_time"] * 60
    self.max_short_open_time = config["max_short_open_time"] * 60
    self.min_short_open_time = config["min_short_open_time"] * 60

    self.state_time = random.gauss(mu=self.inter_duration, sigma=60)
    if settings.DEBUG:
      print("Starting door {} closed for {:.4} minutes".format(self.id(), self.state_time / 60))

  def id(self):
    return self.name

  def x(self):
    return self.area.x + settings.HALF_TILE

  def y(self):
    return self.area.y + settings.HALF_TILE

  def is_open(self):
    return self.open

  def to_next_level(self):
    return self.type == "inter_level"

  def create_area(self, direction):
    w = 0
    h = 0
    area_x = 0
    area_y = 0

    if direction == "up":
      w = settings.TILE_SIZE
      h = DOOR_SIZE
      self.pos.x = self.pos.x - w / 2
      self.pos.y = self.pos.y
      area_x = self.pos.x
      area_y = self.pos.y + h
    elif direction == "down":
      w = settings.TILE_SIZE
      h = DOOR_SIZE
      self.pos.x = self.pos.x - w / 2
      self.pos.y = self.pos.y - h
      area_x = self.pos.x
      area_y = self.pos.y - settings.TILE_SIZE
    elif direction == "left":
      w = DOOR_SIZE
      h = settings.TILE_SIZE
      self.pos.x = self.pos.x
      self.pos.y = self.pos.y - h / 2
      area_x = self.pos.x + w
      area_y = self.pos.y
    elif direction == "right":
      w = DOOR_SIZE
      h = settings.TILE_SIZE
      self.pos.x = self.pos.x - w
      self.pos.y = self.pos.y - h / 2
      area_x = self.pos.x - settings.TILE_SIZE
      area_y = self.pos.y
    else:
       raise Exception("Invalid argument: {}".format(direction))

    area = pygame.Rect(area_x , area_y, settings.TILE_SIZE, settings.TILE_SIZE)

    return [w, h, area]

  def react(self):
    if self.open:
      # Check how long the door has been open
      if self.state_time > 0:
          self.state_time -= 1
          return

      self.state_time = random.gauss(mu=self.inter_duration, sigma=60)
      if settings.DEBUG:
        print("Closing door for {:.4} minutes".format(self.state_time / 60))
      self.open = False
    else:
      # Check how long the door has been closed
      if self.state_time > 0:
          self.state_time -= 1
          return

      # Check if the door should open for a short or long time
      is_short_open = (random.uniform(0, 1) < self.short_opening)
      if is_short_open:
        self.state_time = random.uniform(self.min_short_open_time, self.max_short_open_time)
      else:
        self.state_time = random.uniform(self.min_open_duration, self.max_open_duration)

      if settings.DEBUG:
        print("Opening door for {:.4} minutes".format(self.state_time / 60))

      self.open = True

  def update(self):
    self.react()
    pygame.draw.rect(self.surface, (0, 255, 0) if self.open else (255, 0, 0), self.rect)
    pygame.draw.rect(self.surface, (0, 100, 0) if self.open else (100, 0, 0), self.area)