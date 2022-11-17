from selenium import webdriver
from webdriver_manager.chrome import ChromeDriverManager
import time

driver = webdriver.Chrome(ChromeDriverManager().install())
driver.get("https://berkeley-cs170.github.io/project-leaderboard/?team=MeanPairwiseSocialDistance")

table = dict()
time.sleep(5)

for row in driver.find_elements('xpath', '//tr'):
    cells = row.find_elements('xpath', './/td')
    if len(cells) == 2:
        table[cells[0].text] = cells[1].text

print(table)