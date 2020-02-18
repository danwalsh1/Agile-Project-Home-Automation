import React from 'react';
import Card from 'react-bootstrap/Card';

/*
 * USAGE:
 */

class LightControl extends React.Component{
    constructor(props){
        super(props);
    }

    render() {
        return (
            <div className='lightControl'>
                <Card style={{width: '18rem'}}>
                    <Card.Body>
                        <Card.Title>LIGHT NAME</Card.Title>
                    </Card.Body>
                </Card>
            </div>
        )
    }
}

export default LightControl;