#! /usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'orocos'
include Orocos
Orocos.initialize

Orocos.run 'dps' do
  Orocos.log_all_ports
  dps = TaskContext.get 'dps'

  dps.device = '/dev/ttyS0'

  dps.filter_width = 30;

  dps.configure
  dps.start

  loop do
	sleep 0.01
  end 
end

