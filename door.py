import pygame
import random
import settings

DOOR_SIZE = 2

class Door:
  def __init__(self, id, pos, type, direction, inter_duration, short_opening, max_open_duration, max_close_duration):
    if type != "intra_level" and type != "inter_level":
      raise Exception("Door type {} is not valid".format(type))

    self.open = False
    self.type = type
    self.pos = pos
    self.identifier = id
    self.surface = pygame.display.get_surface()

    w = 0
    h = 0
    area_x = 0
    area_y = 0
    if direction == "up":
      w = settings.TILE_SIZE
      h = DOOR_SIZE
      area_x = pos.x
      area_y = pos.y + h
    elif direction == "down":
      w = settings.TILE_SIZE
      h = DOOR_SIZE
      area_x = pos.x
      area_y = pos.y - settings.TILE_SIZE
    elif direction == "left":
      w = DOOR_SIZE
      h = settings.TILE_SIZE
      area_x = pos.x + w
      area_y = pos.y
    elif direction == "right":
      w = DOOR_SIZE
      h = settings.TILE_SIZE
      area_x = pos.x - settings.TILE_SIZE
      area_y = pos.y
    else:
       raise Exception("Invalid argument: {}".format(direction))

    self.rect = pygame.Rect(pos.x , pos.y, w, h)
    self.area = pygame.Rect(area_x , area_y, settings.TILE_SIZE, settings.TILE_SIZE)

    self.short_opening = short_opening
    self.max_open_duration = max_open_duration * 60      # Minutes to seconds
    self.max_close_duration = max_close_duration * 60
    self.inter_duration = (3 * inter_duration) * 60 - self.max_close_duration # Minutes to seconds

    self.state_time = random.triangular(0, self.max_close_duration, self.inter_duration)

  def id(self):
    return self.identifier

  def x(self):
    return self.area.x + settings.HALF_TILE

  def y(self):
    return self.area.y + settings.HALF_TILE

  def is_open(self):
    return self.open

  def to_next_level(self):
    return self.type == "inter_level"

  def react(self):
    if self.open:
      # Check how long the door has been open
      if self.state_time > 0:
          self.state_time -= 1
          return

      self.state_time = random.triangular(0, self.max_close_duration, self.inter_duration)
      if settings.DEBUG:
        print("Closing door for {:.4} minutes".format(self.state_time / 60))
      self.open = False
    else:
      # Check how long the door has been close
      if self.state_time > 0:
          self.state_time -= 1
          return

      is_short_open = (random.uniform(0, 1) <= self.short_opening)

      # Check if the door should open for a short or long time
      if is_short_open:
        self.state_time = random.uniform(30, 90)
      else:
        self.state_time = random.uniform(90, self.max_open_duration)

      if settings.DEBUG:
        print("Opening door for {:.4} minutes".format(self.state_time / 60))

      self.open = True


  def update(self):
    self.react()
    pygame.draw.rect(self.surface, (0, 255, 0) if self.open else (255, 0, 0), self.rect)
    pygame.draw.rect(self.surface, (0, 100, 0) if self.open else (100, 0, 0), self.area)