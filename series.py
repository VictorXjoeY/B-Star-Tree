# !/usr/bin/env python
# python2
# Gabriel Pinto de Camargo - 9293456
# Gabriel Simmel Nascimento - 9050232
# Marcos Cesar Ribeiro de Camargo - 9278045
# Victor Luiz Roquete Forbes - 9293394
# 02/05/2016

from __future__ import print_function
import urllib2
import re
from sets import Set


initial_url = "http://www.imdb.com/chart/toptv/?ref_=nv_tvv_250_3"
response = urllib2.urlopen(initial_url)
html = response.read()

get_links = re.compile(r'<a href=\"\/title\/(.*?)\"') #getting links
url_list = get_links.findall(html)

qnt = len(url_list) # url_list size

url_set = Set()
for i in range(0, qnt):
	aux = "http://www.imdb.com/title/" + url_list[i] # adding the initial part of url
	url_set.add(aux) # adding a set, to remove the repetition

qnt = len(url_set) 
url_list = []
for i in range(0, qnt):
	url_list.append(url_set.pop()) # copying again to list

for i in range(0, qnt):
	response = urllib2.urlopen(url_list[i])
	html = response.read()
	# title
	get = re.compile(r'.*<meta property=\'og:title\' content="(.*?) \(.*')

	titles = get.findall(html)
	print ("title=\""+ titles[0] +"\"")
	# #description
	get = re.compile(r'<meta name=\"description\" content=\".*\. (.*?)\" />')
	description = get.findall(html)
	print ("description=\""+ description[0] +"\"")

	# # year
	get = re.compile(r'.*year=(.{4}?)&ref.*"')
	year = get.findall(html)
	print ("year=\"" +str(min(year)) +"\"")

	# # season
	get = re.compile(r'.*season=(.*?)&ref.*"')
	season = get.findall(html)
	print ("season=\"" +str(max(season)) +"\"")

	# # genre
	get = re.compile(r'.*itemprop=\"genre\">(.*?)</span>.*')
	genre = get.findall(html)
	print ("genre=\""+genre[0], end='')
	for j in range(1, len(genre)):
		print (", "+genre[j], end='')
	print ("\"")

	# # production
	get = re.compile(r'<a href=\"/country/.*\"\nitemprop=\'url\'>(.*?)</a>')
	production = get.findall(html)
	print ("production=\""+production[0], end='')
	for j in range(1, len(production)):
		print (", "+production[j], end='')
	print ("\"")

	print ("")
