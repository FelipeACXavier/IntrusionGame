import pygame
import random
import settings

class Entity:
    def __init__(self, pos, color):
      self.surface = pygame.display.get_surface()
      self.direction = pygame.math.Vector2()
      self.color = color

      self.pos = pos
      self.is_checking = False
      self.speed = settings.TILE_SIZE

    def x(self):
        return self.pos.x

    def y(self):
        return self.pos.y

    def start_check(self):
      self.is_checking = True

    def stop_check(self, speed=None):
      self.is_checking = False

    def is_being_checked(self):
      return self.is_checking

    def adjust_pos(self, speed = None):
      self.direction.x = random.randint(-1, 1)
      self.direction.y = random.randint(-1, 1)

      if speed == None:
        speed = 1.2 * self.speed

      x = (self.direction.x * speed)
      y = (self.direction.y * speed)

      self.pos.x += x
      self.pos.y += y

      self.constrain(speed)

    def constrain(self, speed):
      if self.pos.x > settings.WIDTH or self.pos.x < 0:
        self.pos.x += -(self.direction.x * speed)

      if self.pos.y >settings. HEIGHT or self.pos.y < 0:
        self.pos.y += -(self.direction.y * speed)

    def move(self, speed):
      if self.is_checking:
        return

      self.direction.x = random.uniform(-1, 1)
      self.direction.y = random.uniform(-1, 1)

      self.pos.x += (self.direction.x * speed)
      self.pos.y += (self.direction.y * speed)

      self.constrain(speed)

    def update(self):
      self.move(self.speed)
      if settings.FPS <= 120:
        pygame.draw.circle(self.surface, self.color, self.pos, settings.HALF_TILE * 0.5)