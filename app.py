from flask import Flask, request, render_template
from config import Config
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from werkzeug.security import generate_password_hash, check_password_hash
import json


app = Flask(__name__)
app.config.from_object(Config)
db = SQLAlchemy(app)
migrate = Migrate(app, db)

#flask db init
# flask db migrate
#flask db upgrade
#psql -d kynetech-dev
#psql \q

# https://gist.github.com/mayukh18/2223bc8fc152631205abd7cbf1efdd41/
# https://realpython.com/flask-by-example-part-2-postgres-sqlalchemy-and-alembic/

# class User(db.Model):
#     """A general user capable of ranking clubs.
#
#     :param str email: email address of user
#     :param str password_hash: encrypted password for the user
#
#     """
#     id = db.Column(db.Integer, primary_key=True)
#     username = db.Column(db.String(64), index=True, unique=True)
#     password_hash = db.Column(db.String(128))
#     rankings = db.relationship('Ranking', backref='author', lazy='dynamic')
#
#     # helper method for printing
#     def __repr__(self):
#         return '<User {}>'.format(self.username)
#
#     def set_password(self, password):
#         self.password_hash = generate_password_hash(password)
#
#     def check_password(self, password):
#         return check_password_hash(self.password_hash, password)
#
class Entry(db.Model):
    __tablename__ = 'entries'
    id = db.Column(db.Integer, primary_key=True)
    data = db.Column(db.String())

    def __init__(self, data):
        self.data = data

    def __repr__(self):
        return '<id {}>'.format(self.id)

# clubs = []
# with open('club_list.json') as f:
#     clubs = json.load(f)

@app.route('/')
def main():
    # if len(User.query.all()) == 0:
    #     jennifer = User(username='jennifer')
    #     db.session.add(jennifer)
    #     db.session.commit()
    #
    # if len(Ranking.query.all()) == 0:
    #     jennifer_ranking = Ranking(ranking_jsonstr=str(clubs), author=jennifer)
    #     db.session.add(jennifer_ranking)
    #     db.session.commit()
    all = Entry.query.order_by(Entry.id).all()
    d = [e.data for e in all]
    return render_template('index.html', data = d)

@app.route('/data', methods=['GET', 'POST'])
def data():
    file = open("data.txt", "r")
    data = [line.strip('\n').split(',') for line in file]
    try:
        data = [[int(j) for j in i] for i in data]
    except:
        pass
    print(data)
    file.close()
    all = Entry.query.order_by(Entry.id).all()
    d = [e.data for e in all]
    return str(d)

@app.route('/test', methods=['GET', 'POST'])
def test():
    file = open("data.txt", "w")
    file.write("1,2,3"+ "\n")
    print("1,2,3")
    file.close()
    entry = Entry(data=str("1,2,3"))
    db.session.add(entry)
    db.session.commit()
    return str("1,2,3") + " added!"
    
@app.route('/delete', methods=['GET', 'POST'])
def delete():
    db.session.query(Entry).delete()
    db.session.commit()
    return "deleted all entries"

@app.route('/post', methods = ["POST"])
def post():
    file = open("data.txt", "w")
    file.write(str(request.data.decode('utf-8'))+ "\n")
    print(request.data.decode('utf-8'))
    file.close()
    entry = Entry(data=str(request.data.decode('utf-8')))
    db.session.add(entry)
    db.session.commit()
    return str(request.data.decode('utf-8')) + " added!"

# @app.route('/api')
# def api():
#     return "Welcome to the PennClubReview API!."
#
# @app.route('/api/clubs', methods=['GET'])
# def get_clubs():
#     return str(clubs)
#
# @app.route('/api/clubs', methods=['POST'])
# def create_club():
#     if request.json:
#         club = {
#             'name': request.json['name'],
#             'size': request.json['size'],
#         }
#         clubs.append(club)
#         json.dump(clubs, 'club_list.json')
#         return str(clubs)
#
# @app.route('/api/rankings', methods=['GET'])
# def get_rankings():
#     if len(User.query.all()) != 0:
#         jennifer = User.query.get(1)
#         jennifer_rankings = eval(jennifer.rankings[0].ranking_jsonstr)
#         print(jennifer_rankings)
#         return jennifer.rankings[0].ranking_jsonstr
#
#
# @app.route('/api/rankings', methods=['POST'])
# def change_rankings():
#     if request.json:
#         jennifer = User.query.get(1)
#         jennifer_rankings = eval(jennifer.rankings[0].ranking_jsonstr)
#
#         for i in range(len(jennifer_rankings)):
#             if jennifer_rankings[i]['name'] == request.json['name']:
#                     jennifer_rankings[i] = {jennifer_rankings[i]['name']: str(request.json['size'])}
#
#         return str(jennifer_rankings)
#
# @app.route('/user/<int:user_id>')
# def user(user_id):
#     # Do something with user_id
#     pass

if __name__ == '__main__':
    app.run()
